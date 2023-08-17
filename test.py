import socket, struct, time

s = socket.socket()

s.connect(("127.0.0.1", 55555))

s.send("\0\0\0\0\0\0\0\0\0\0\0\0\0\0".encode())
print(s.recv(4096))
print(s.recv(4096))

s.close()