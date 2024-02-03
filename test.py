import json, socket, struct, ctypes
from threading import Thread

def stringhash(data: bytes) -> int:
    hashnum = ctypes.c_uint64(14695981039346656037)
    for i in range(len(data)):
        hashnum.value ^= int(data[i])
        hashnum.value *= 1099511628211
    
    return hashnum.value

def sendPack(s: socket.socket, msg: bytes):
    s.send(struct.pack(">i Q i i Q {}s".format(len(msg) + 2), 30 + len(msg), 123123, 1, -1, stringhash(msg), msg + b'\0\0'))

#sendPack(json.dumps({"function":"login", "parameters": {"user_id": 10000, "password": "123456"}}).encode())
def run():
    s = socket.socket()
    s.connect(("127.0.0.1", 55555))
    sendPack(s, b"test")
    for i in range(10000):
        sendPack(s, json.dumps({"function":"register", "parameters": {"email": "2098332747@qq.com", "password": "123456"}}).encode())
        getdata = s.recv(1024)
        print(getdata)
        print(struct.unpack(">i", getdata[:4]), len(getdata))

for i in range(10):
    Thread(target=run).start()
    
# s = socket.socket()
# s.connect(("127.0.0.1", 55555))
# sendPack(s, b"test")

# sendPack(s, json.dumps({"function":"register", "parameters": {"email": "2098332747@qq.com", "password": "123456"}}).encode())
# getdata = s.recv(1024)
# print(getdata)
# print(struct.unpack(">i", getdata[:4]), len(getdata))

# sendPack(s, json.dumps({"function":"login", "parameters": {"user_id": 10000, "password": "123456"}}).encode())
# getdata = s.recv(1024)
# print(getdata)
# print(struct.unpack(">i", getdata[:4]), len(getdata))