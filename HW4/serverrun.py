import os
with open("list.txt", "r") as f:
    for i in range(0,101):
        line = f.readline()
        line = line[-6:-1]
        if len(line)>3:
            cmd = "./server " + line
            os.popen(cmd)