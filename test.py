import json, socket, struct, ctypes

def stringhash(data: bytes) -> int:
    hashnum = ctypes.c_uint64(14695981039346656037)
    for i in range(len(data)):
        hashnum.value ^= data[i]
        hashnum.value *= 1099511628211
    
    return hashnum.value

s = socket.socket()
s.connect(("127.0.0.1", 55555))
s.send(struct.pack("<i Q i i Q 3s", 31, 0, 0, -1, stringhash(b'a'), 'a\0\0'.encode()))
s.send(struct.pack("<i Q i i Q 3s", 31, 123123, 1, -1, stringhash(b'a'), 'a\0\0'.encode()))
print(s.recv(1024))