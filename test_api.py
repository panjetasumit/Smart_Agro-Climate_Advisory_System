import requests

url = "http://127.0.0.1:5000/predict"
data = {
    "pressure": 1015.9,
    "dewpoint": 19.9,
    "humidity": 95,
    "cloud": 81,
    "sunshine": 0.0,
    "winddirection": 40.0,
    "windspeed": 13.7
}

response = requests.post(url, json=data)
print(response.json())
