import socket
import selectors

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print ("Socket successfully created")
except socket.error as err:
    print ("socket creation failed with error %s" %(err))
#add a decent error handling
port = 5000
s.connect(('127.0.0.1', port))

s_manager = selectors.DefaultSelector()
s_manager.register(s, selectors.EVENT_READ)

# print(type(s_manager.select(0)))
while True:
    events = s_manager.select(None)
    print('unblocked')
    if events != []:
        #handle events
        key, data = events[0]

        print(key.fileobj.recv(1025))
