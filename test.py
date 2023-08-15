import socket, struct, time

s = socket.socket()

s.connect(("127.0.0.1", 55555))

ss = "你好".encode()
data = struct.pack("I {}s".format(len(ss)), len(ss), ss)

s.send(data)
print(s.recv(1024))

s.close()