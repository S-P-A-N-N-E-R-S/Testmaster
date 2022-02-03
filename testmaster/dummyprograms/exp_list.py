import sys

# n = 28 is pretty big already 268,435,456 list entries and takes a few seconds
n = int(sys.argv[1])
l1 = [1]
for i in range(0,n):
    l1 = l1 + l1
print("2 ^", n, "=", len(l1))
