#Calculate the increment of a number in a very slow way
import sys 
n = int(sys.argv[1])
diff = 1
  
for i in range(1,n-1):
    diff += 1

print (n, "- 1 =", diff)