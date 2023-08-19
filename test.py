import requests

s = requests.Session()

r = s.post("https://testapi.hcolda.com/api.php?type=server&commend=aeskey", {"serverid":'', 'serverkey':'', 'uuid':'225BD9F9-493D-F9E5-50A4-9BE0C59FB859'})
print(r.json())