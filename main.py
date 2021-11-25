import requests
import serial

my_token_id = "7b7c27047351d47a2413256f2d69c3f38bbfee92"
my_city = "Frankfurt"

url = f"https://api.waqi.info/feed/{my_city}/?token={my_token_id}"

web_data = requests.get(url)

json_format = web_data.json()

aqi = json_format['data']['aqi']

serial_session = serial.Serial('COM3', 9600)
if not serial_session.is_open:
    serial_session.open()
serial_session.write(str(aqi).encode())
