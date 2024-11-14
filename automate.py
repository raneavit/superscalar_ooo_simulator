import sys, os
import shlex
from subprocess import Popen, PIPE

# sys.stdout = open('output.txt', 'w')

for i in range(3, 8):
    for j in range(0, 4):
        command = 'cmd /c "start sim 512 ' + str(2^i) + ' ' + str(2^j) +  ' val_trace_perl1 > output' + str(2^i) + '_' + str(2^j) +'.txt" '
        # print(command)
        os.system(command)

# sys.stdout.close()



