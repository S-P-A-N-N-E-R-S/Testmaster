import sys
def fibonacci(n):
    if n in {0, 1}:  # Base case
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

x = int(sys.argv[1])
print ("fibonacci von", x, "=", fibonacci(x))