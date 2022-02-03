import json
import sys

n = int(sys.argv[1])
l1 = [1]
for i in range(0,n):
    l1 = l1 + l1
    
output = {
    "command": "algoid231223",  # optional key
    "status": "Success",
    "runtime": 1234,
    "weight": 342,
    "actual_stretch": 1.523,
    "graph_information": {
        "nodes": 231,
        "edges": 1000,
        "directed": False,
        "weighted": True,
        "simple": True
    },
    "additional_info": {"delta": 1.23},  # optional key?
}
print(json.dumps(output))
