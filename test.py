import socket
import sys

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print ("Socket successfully created")
except socket.error as err:
    print ("socket creation failed with error %s" %(err))

port = 5000
s.connect(('127.0.0.1', port))
s.setblocking(0)
while True:

    try:


        print(str(s.recv(1024)))
        s.send(bytes("close", 'utf-8'))
        break
    except:
        continue
