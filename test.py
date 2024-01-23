import json, socket, struct, ctypes

def stringhash(data: bytes) -> int:
    hashnum = ctypes.c_uint64(14695981039346656037)
    for i in range(len(data)):
        hashnum.value ^= int(data[i])
        hashnum.value *= 1099511628211
    
    return hashnum.value

s = socket.socket()
s.connect(("127.0.0.1", 55555))
print(stringhash(b'test'))
s.send(struct.pack(">i Q i i Q 6s", 31+3, 0, 0, -1, stringhash(b'test'), 'test\0\0'.encode()))
s.send(struct.pack(">i Q i i Q 6s", 31+3, 123123, 1, -1, stringhash(b'test'), 'test\0\0'.encode()))
getdata = s.recv(1024)
print(struct.unpack(">i", getdata[:4]), len(getdata))