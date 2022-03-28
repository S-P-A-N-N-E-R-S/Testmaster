# Testmaster - an application for automated testing

This application can schedule and run many programs (e.g. from terminal) at once while controlling memory and time limits for each iteration and is thus useful for usages such as algorithm engineering for scheduling longer experiments on many instances.

### Disclaimer
The json-parser is written to suit the experiments done by the S.P.A.N.N.E.R.S. project group and therefore produces a useful output file on specific program outputs for now. For general usage, some tweakments are required, especially in the `parse_command_data`, `check_json_keys`, etc functions. Other than that the parallel processing works on various programs.

### Testmaster usage 

* Minimum required version of python: **3.7**
* Execute testmaster with an input file of the following format:

##### Structure
```json
{
    "time_limit" : [int in ms],
    "memory_limit" : [int in MB],
    "number_processes" : [int],
    "commands" : [ [string list of command line commands] ],
    "output_filename" : [string],
}
```

* The list of commandline commands must contain an array of commands of your tests.
* Your applications output must be to stdout in a json format too to be processed. In case you don't want/have/need the values, just add any valid json value.
##### Output Structure
```json
{
    "command": "...",  # optional
    "status": [string],
    "runtime": [int],
    "weight": [float],
    "actual_stretch": [float],
    "graph_information": {
        "nodes": [int],
        "edges": [int],
        "directed": [bool],
        "weighted": [bool],
        "simple": [bool]
    },
    "additional_info": { [whatever] },  # optional
}
```

* The commands are executed in parallel. The number of processes can be limited with the "number_processes" value.
* After a the given `time_limit` or `memory_limit` on a single command logs an error message and continues on working on the other commands
* In case something bad happens or an unexpected error occurd (or KeyboardInterrupt)the Testmaster should still produce an output file with the finished processes. 
* Before running a three hour long testing session I recommend testing with a few small examples to check if the output is recognized correctly.
* You can test the program with a an example imput running processes of some dummy functions

```python3 testmaster.py -i "example_input.json"```
