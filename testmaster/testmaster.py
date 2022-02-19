import multiprocessing as mp
import resource
import subprocess as sp
import json
import sys, getopt
from time import time, sleep
import threading as thr
from queue import Queue

# Font utility class for formattet output
class font:
   PURPLE = '\033[95m'
   CYAN = '\033[96m'
   DARKCYAN = '\033[36m'
   BLUE = '\033[94m'
   GREEN = '\033[92m'
   YELLOW = '\033[93m'
   RED = '\033[91m'
   BOLD = '\033[1m'
   UNDERLINE = '\033[4m'
   END = '\033[0m'


# If this is true, a bunch of debug print statements are printed
DEBUG = True


def process_input(json_filename):
    """
    Utility function that parses the input from a json file to a dictionary
    @param json_filename Name of the input file
    @return A dictionary that is parsed from the json file
    """
    data = 1
    try:
        with open(json_filename) as json_file:
            data = json.load(json_file)
            return data
    except FileNotFoundError:
        print("Input file does not exist.")
        print_usage()
        sys.exit(2)


def get_input_file(argv):
    """
    Processes the arguments given to the program.
    @param argv Must be system arguments
    @return The name of the input file
    """
    input_file = ''
    try:
        opts, args = getopt.getopt(argv,"hi:o:d",["ifile=","ofile="])
    except getopt.GetoptError:
        print_usage()
        sys.exit(2)
    if len(sys.argv) == 1:
        print_usage()
        sys.exit()
    for opt, arg in opts:
        if opt == '-h' or opt == "-help":
            print_usage()
            sys.exit()
        elif opt in ("-i", "--ifile"):
            input_file = arg
        elif opt == "-d":
            global DEBUG
            DEBUG = False

    return input_file


def print_usage(complete=True):
    """
    A nice generated welcome message when the Testmaster is used with incorrect or missing keywords.
    """
    if complete:
        print(font.BOLD + font.CYAN + "Welcome to Testmaster" + font.END + font.END)
        print("Provide an input file in the following way:")
        print("testmaster.py", font.BOLD + "-i" + font.END, "[input_file_name].json",)
        print("To disable the output set flag -d.")
    print("The required format of the json-file is specified here:")
    print(font.BLUE + "https://gitpgtcs.informatik.uni-osnabrueck.de/spanners/experiments/-/tree/testmaster" + font.END)


def check_json_keys(json_dict):
    """
    Checks if all required keys are set
    :param json_dict: dict parsed from json
    :return: True if required key are set
    """
    required_keys = ["command", "runtime", "weight", "actual_stretch", "graph_information"]
    required_graph_information = ["nodes", "edges", "directed", "weighted", "simple"]

    for key in required_keys:
        if key not in json_dict:
            return False

    for key in required_graph_information:
        if key not in json_dict["graph_information"]:
            return False

    return True


def parse_command_data(cmd, data):
    """
    Parses the stdout and stderr data of a command
    :param cmd: Command related to the data
    :param data: (stdout_data, stderr_data)
    :type data: tuple
    :return: output json string
    """
    output, error = data

    error_dict = {"command": cmd}

    # error in subprocess
    if error:
        # C++ should throw a "Memory Limit Exceeded" error and Python "MemoryError" if memory limit is exceeded
        if "Memory" in error:
            error_dict["error"] = "Memory limit exceeded."
        else:
            error_dict["error"] = error
        return json.dumps(error_dict)

    # parse json output
    try:
        parsed_output = json.loads(output)

        # add command if not included
        if "command" not in parsed_output:
            parsed_output["command"] = cmd

        if check_json_keys(parsed_output):
            return json.dumps(parsed_output)
        else:
            error_dict["error"] = "Please provide all required keys in the json output."
            error_dict["output"] = output
        return json.dumps(error_dict)
    except json.JSONDecodeError:
        error_dict["error"] = "The command output can't be parsed. Ensure that the output is formatted in json."
        error_dict["output"] = output
        return json.dumps(error_dict)

def create_output_str(json_output, i):
    """
    Creates an output string that allows for the json output strings to be parsed into a big output file
    @param json_output string of one single json output of a process
    @param i index of the process
    """
    return ',\n' + '"Test' + str(i) + '" : ' + json_output


def work(queue, lock, time_limit, memory_limit, output_queue):
    """
    Worker function which starts a subprocess and handles time and memory limit.
    Handles writing the result to the output file.
    @param queue The queue to pop new jobs from
    @param output_file A global output file where all finished jobs write to
    @param time_limit A global time limit for all jobs in ms
    @param memory_limit A global memory limit for all jobs in MB
    """

    # calculate the memory limit. It is passed as MB so it needs to be calculated to byte first
    def set_mem_limit():
        ml = memory_limit * 1000000
        resource.setrlimit(resource.RLIMIT_AS, (ml, resource.RLIM_INFINITY))

    while True:

        lock.acquire()
        if queue.empty():
            lock.release()
            break
        task_index, cmd = queue.get()
        lock.release()

        try:
            commands = cmd.split()
            proc = sp.Popen(commands, shell=False, stdout=sp.PIPE, stderr=sp.PIPE, text=True, preexec_fn=set_mem_limit)
            try:
                # Pre function set_mem_limit limits the memory that is used for one job
                # Since the queue items are a tuple with an index, access with indexing on [1]
                proc.wait(timeout=time_limit/1000)  # ms to second

                # parse command data
                json_output = parse_command_data(cmd, proc.communicate())

                if DEBUG:
                    print("Test", task_index, "finished.", cmd)
                output_queue.put(create_output_str(json_output, task_index))

            except sp.TimeoutExpired as e:
                json_output = json.dumps({"command": cmd, "error": "Time limit exceeded."})
                if DEBUG:
                    print("Test", task_index, "didn't finish.", "Timeout expired.", cmd)
                output_queue.put(create_output_str(json_output, task_index))

            finally:
                proc.kill()

        except Exception as e:
            json_output = json.dumps({"command": cmd, "error": e})
            if DEBUG:
                print("Test", task_index, "didn't finish.", "Error occurred.", cmd)
            output_queue.put(create_output_str(json_output, task_index))


class OutputThread(thr.Thread):
    """
    A class for the daemon to thread securely write the results of the finished processes while the testmaster is running
    @param output_queue A queue where the output strings are stored
    @param output_file The target output file where the thread is supposed to write to
    """

    def __init__(self, output_queue, output_file):
        super().__init__(daemon=True)

        self.output_queue = output_queue
        self.output_file = output_file

        self._thread_stopped = thr.Event()

    def stop(self):
        self._thread_stopped.set()

    def stopped(self):
        return self._thread_stopped.is_set()

    def run(self):
        while True:
            if not self.output_queue.empty():
                with open(self.output_file, 'a') as f:
                    f.write(self.output_queue.get())

            if self.stopped():
                break

            sleep(1)
            # if DEBUG:
            print("daemon still running!")


if __name__ == "__main__":

    # Get the input file specified and extract the parameters from the dictionary
    input_file = get_input_file(sys.argv[1:])
    input = process_input(input_file)
    time_limit = input["time_limit"]
    memory_limit = input["memory_limit"]
    number_processes = input["number_processes"]
    commands = input["commands"]
    output_filename = input["output_filename"]

    print(font.BOLD + font.CYAN + "Welcome to Testmaster" + font.END + font.END)
    print("Running some tests now. output =", DEBUG, "\n")

    # Check if the input file is ok
    if not all(isinstance(i, int) for i in [time_limit, memory_limit, number_processes]):
        print("Error in input file: time_limit, memory_limit and number_processes must be integers.")
        print_usage(False)
        exit(1)
    if not type(commands) == list:
        print("Error in input file: commands must be a list of shell commands")
        print_usage(False)
        exit(1)
    if not type(output_filename) == str:
        print("Error in input file: output_filename must be a string containing a valid file path")
        print_usage(False)
        exit(1)

    if DEBUG:
        print(font.BOLD + 'Input file:' + font.END, input_file)
        print(font.BOLD + "Metadata:" + font.END,
            str(time_limit)+"ms,",
            str(memory_limit)+"MB,",
            str(number_processes)+" processes,",
        )
        print(font.BOLD + "Commands:" + font.END,)
        for i, c in enumerate(commands):
            print(str(i) + ".", font.GREEN + c + font.END)
        print(font.BOLD + "Write to:" + font.END, output_filename)

    # Calculate the max number of CPUs that can be used. Use n-1 to ensure normal computer activities are still running
    n_cpus = mp.cpu_count() - 1
    if DEBUG:
        if number_processes > n_cpus:
            print(font.BOLD + font.RED + "Info:" + font.END + font.END, "Less CPUs are available than specified.")
        print(font.BOLD + "Max Number of CPUs:" + font.END, n_cpus)

    # If less processes are wanted, set this here
    if number_processes < n_cpus:
        n_cpus = number_processes
    if DEBUG:
        print(font.BOLD + "Used CPUs:" + font.END, n_cpus,"\n")

    # Output queue for the finished processes output. Will write to file no matter what happens in the processes
    output_queue = Queue()

    # Add an initial value to the file in order to secure correct json-file output
    f = open(output_filename, 'w')
    f.write('{"initialized" : true')
    f.close()

    # Measure the time the processes take because why not, also nice for debugging!
    t1 = time()

    # Start a daemon thread that is able to write to file while the processes are still running
    output_thread = OutputThread(output_queue, output_filename)
    output_thread.start()

    # To execute in parallel a queue is provided.
    # From the queue the processes are started as soon as there's space.
    # The number of processes is limited by the user.
    try:
        proc_queue = mp.Queue(maxsize=len(commands))
        # Adding an index to the command to keep track of the commands ids
        for i, cmd in enumerate(commands):
            proc_queue.put((i, cmd))
        active_processes = []
        lock = mp.Lock()
        for proc in range(number_processes):
            active_processes.append(thr.Thread(target=work, args=(proc_queue, lock, time_limit, memory_limit, output_queue)))
            active_processes[proc].start()
        # Wait for the processes/threads to finish
        for proc in active_processes:
            proc.join()

    except KeyboardInterrupt:
        print("Tests manually aborted. Some tests might not be finished.")
    except:
        print(font.RED + "An unexpected error occurred." + font.END)
        print("Sorry about that. The finished processes should be written to the specified output file anyway ... if not please contact the developer(s).")
    finally:
        # Stop file output thread
        output_thread.stop()
        output_thread.join()
        # Write the rest of the output queue to the file
        # If something goes wrong in the program the queue remaining content will be written to the file eventually
        f = open(output_filename, 'a')
        while not output_queue.empty():
            f.write(output_queue.get())
        f.write("}")
        f.close()

        # Calculate the time the program used to do all the processes. 
        # Has no specific use except a nice output at the end of the Testmaster
        t2 = time()
        count_time = int(t2-t1)
        if count_time < 60:
            print(font.CYAN + font.BOLD + "\nTestmaster worked for approx.", count_time, "seconds." + font.END + font.END)
        else:
            count_time = round(count_time / 60, 2)
            print(font.CYAN + font.BOLD + "\nTestmaster worked for approx.", count_time, "minutes." + font.END + font.END)

        print(font.CYAN + "Finished writing to file!" + font.END)
        print("The output can be found in", font.BOLD + output_filename + font.END)
