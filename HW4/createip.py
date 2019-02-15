f = open('list.txt', 'w')
f.truncate()
for i in range(8001,8101):
    s = "127.0.0.1    " + str(i) + "\n"
    f.writelines(s)
f.close()