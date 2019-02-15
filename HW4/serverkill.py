# import subprocess
import os 
port_list = list()
with open("list.txt", "r") as f:
    for i in range(0,101):
        line = f.readline()
        line = line[-5:-1]
        if line:
            port_list.append(int(line))
            
from subprocess import Popen, PIPE

def run(command):
    process = Popen(command, stdout=PIPE, shell=True, universal_newlines=True)
    while True:
        line = process.stdout.readline().rstrip()
        if not line:
            break
        yield line


if __name__ == "__main__":
    for port in port_list:
        askpid = "lsof -i -P -n | grep " + str(port)
        for path in run(askpid):
            pid = path[10:16]
            print("killing %s ..."%(pid))
            s = "kill " + str(pid)
            os.popen(s)
