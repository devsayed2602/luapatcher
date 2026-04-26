import requests
import json

url = 'https://webserver-ecru.vercel.app/api/social/chat/send'
headers = {'Content-Type': 'application/json'}
data = {
    "sender": "empower",
    "receiver": "leVi",
    "message": "hello from script"
}

response = requests.post(url, headers=headers, json=data)
print(f"Status Code: {response.status_code}")
try:
    print(response.json())
except:
    print(response.text)
