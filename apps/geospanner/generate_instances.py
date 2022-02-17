# quick and dirty input generation
import math
import json
import sys

input = {}

threads = 15
timelimit = 600000 #MS
mem_lim = 15000 #MB

input["time_limit"] = timelimit
input["memory_limit"] =  mem_lim
input["number_processes"] = threads
input["output_filename"] = sys.argv[2]

prefix = "./build/apps/geospanner/"

no_nodes = [100, 1000, 5000, 10000, 150000, 20000]
no_instances = 2

metrics = ["euclid", "sphere"]

classes = ["uniform"]

stretchs = [1.5]

algorithms = ["path_greedy_spanner", "yao_spanner", "delta_greedy_spanner", "yao_spanner_pruning", "yao_parametrized_pruning"]

bounding_box_euclid = math.pi

commands = []

for metric in metrics:
    for c in classes:
        for n in no_nodes:
            for s in stretchs:
                for algo in algorithms:
                    for i in range(no_instances): #i will act as seed
                        command = prefix + algo + " " + str(s) + " "
                        if algo == "delta_greedy_spanner":
                            delta = math.sqrt(s)
                            command += str(delta) + " "
                        elif algo == "yao_parametrized_pruning":
                            yao_t = math.sqrt(s)
                            command += str(yao_t) + " "
                        command += metric + " " + c + " " + str(i) + " " + str(n)
                        if metric=="euclid":
                            command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                        commands.append(command)
                        
input["commands"] = commands

inputfile = sys.argv[1]
with open(inputfile, "w") as f:
    f.write(json.dumps(input, indent=4, sort_keys=False))
