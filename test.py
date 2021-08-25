import socket
import selectors
import time

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print ("Socket successfully created")
except socket.error as err:
    print ("socket creation failed with error %s" %(err))
#add a decent error handling
port = 6000
s.connect(('127.0.0.1', port))

s_manager = selectors.DefaultSelector()
s_manager.register(s, selectors.EVENT_READ)
start_time = time.time()
bufer =""
# print(type(s_manager.select(0)))
while True:
    if time.time() - start_time > 5.0:
        s.send("pepino".encode())
        start_time = time.time()

    events = s_manager.select(0)

    if events != []:
        #handle events
        key, data = events[0]
        bufer = key.fileobj.recv(256).decode('utf-8')
