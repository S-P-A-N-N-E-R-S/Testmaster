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

no_nodes = [1000]
no_clusters = [10, 25, 100]
spread = math.pi/4
no_instances = 50

metrics = ["euclid", "sphere"]

stretchs = [1.5]
yao_pres = [1.1, 1.2, 1.3, 1.4]
deltas = [1.0, 1.1, 1.2, 1.3, 1.4, 1.5]

algorithms = ["path_greedy_spanner", "yao_spanner", "delta_greedy_spanner", "yao_spanner_pruning", "yao_parametrized_pruning"]

bounding_box_euclid = math.pi

commands = []

##clustered stuff
for metric in metrics:
        for n in no_nodes:
            for s in stretchs:
                for c in no_clusters:
                    for algo in algorithms:
                        for i in range(no_instances): #i will act as seed
                            for delta in deltas:
                                if algo == "delta_greedy_spanner":
                                    command = prefix + algo + " " + str(s) + " "
                                    command += str(delta) + " "
                                    command += metric + " " + "cluster" + " " + str(i) + " " + str(c) + " "
                                    nodes_per_cluster = int(n/c)
                                    command += str(nodes_per_cluster) + " " + str(spread)
                                    if metric=="euclid":
                                        command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                                    commands.append(command)
                            for yao in yao_pres:
                                if algo == "yao_parametrized_pruning":
                                    command = prefix + algo + " " + str(s) + " "
                                    command += str(yao) + " "
                                    command += metric + " " + "cluster" + " " + str(i) + " " + str(c) + " "
                                    nodes_per_cluster = int(n/c)
                                    command += str(nodes_per_cluster) + " " + str(spread)
                                    if metric=="euclid":
                                        command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                                    commands.append(command)
                            if (not algo == "yao_parametrized_pruning") and (not algo == "delta_greedy_spanner"):
                                command = prefix + algo + " " + str(s) + " "
                                command += metric + " " + "cluster" + " " + str(i) + " " + str(c) + " "
                                nodes_per_cluster = int(n/c)
                                command += str(nodes_per_cluster) + " " + str(spread)
                                if metric=="euclid":
                                    command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                                commands.append(command)

#uniform stuff:
for metric in metrics:
        for n in no_nodes:
            for s in stretchs:
                for algo in algorithms:
                    for i in range(no_instances): #i will act as seed
                        for delta in deltas:
                            if algo == "delta_greedy_spanner":
                                command = prefix + algo + " " + str(s) + " "
                                command += str(delta) + " "
                                command += metric + " " + "uniform" + " " + str(i) + " " + str(n)
                                if metric=="euclid":
                                    command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                                commands.append(command)
                        for yao in yao_pres:
                            if algo == "yao_parametrized_pruning":
                                command = prefix + algo + " " + str(s) + " "
                                command += str(yao) + " "
                                command += metric + " " + "uniform" + " " + str(i) + " " + str(n)
                                if metric=="euclid":
                                    command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                                commands.append(command)
                        if (not algo == "yao_parametrized_pruning") and (not algo == "delta_greedy_spanner"):
                            command = prefix + algo + " " + str(s) + " "
                            command += metric + " " + "uniform" + " " + str(i) + " " + str(n)
                            if metric=="euclid":
                                command += " " + str(bounding_box_euclid) + " " + str(bounding_box_euclid)
                            commands.append(command)
                    
input["commands"] = commands

inputfile = sys.argv[1]
with open(inputfile, "w") as f:
    f.write(json.dumps(input, indent=4, sort_keys=False))
