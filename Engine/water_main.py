import serial
from datetime import datetime
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import certifi
import time

# InfluxDB Connection
token = '8Kph_oFbnNqTvgUfERhTJveJpw1A08-HDsnhftEIYj8HRSUJSmKIdoPZdaIbNeIkG4likhDrOtr5FgyF5W03hg=='
org = "Ethan"
bucket = "Test"  # Use "bucket" instead of "database" in influxdb-client
url = "https://us-east-1-1.aws.cloud2.influxdata.com"

# Create InfluxDB client
client = InfluxDBClient(url=url, token=token, org=org, ssl_ca_cert=certifi.where())
write_api = client.write_api(write_options=SYNCHRONOUS)

# File paths for confrimation/ error logging
sensor_log_file_path = '/home/openstem/Desktop/openstem_ethan/water_sensor_log.txt'
pump_log_file_path = '/home/openstem/Desktop/openstem_ethan/pump_control_log.txt'

# Establish wired connection to Arduino
ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
time.sleep(2)

# Function to log messages
def log_message(message, log_file_path):
    with open(log_file_path, 'a') as log_file:
        log_file.write(f"{datetime.now().strftime('%Y-%m-%d %H:%M:%S')} - {message}\n")

# DATA UPLOADING
# Function to fetch sensor data from Arduino
def fetch_sensor_data():
    ser.write(b'R')  # Send 'R' command to request sensor readings
    time.sleep(5)  # Wait for Arduino to respond

    # Retrieve data printed to Arduino console
    data = {"pH": None, "EC": None, "Temperature": None}
    while ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        if "pH" in line:
            data["pH"] = float(line.split(":")[1].strip())
        elif "EC" in line:
            data["EC"] = float(line.split(":")[1].strip())
        elif "Temperature" in line:
            data["Temperature"] = float(line.split(":")[1].strip())
    return data

# Function to upload water sensor data to InfluxDB
def upload_water_data(data):
    if all(value is not None for value in data.values()):
        point = (
            Point("water_sensors")  # Measurement for water sensors
            .field("RTD", data["Temperature"])  # RTD value
            .field("pH", data["pH"])  # pH value
            .field("EC", data["EC"])  # Electrical conductivity
        )
        write_api.write(bucket=bucket, org=org, record=point)
        log_message("Data uploaded to InfluxDB", sensor_log_file_path)
    else:
        log_message("Incomplete data - Upload aborted.", sensor_log_file_path)

# PUMP CONTROL LOGIC
# Function to trigger pump control on Arduino
def trigger_pump_control():
    try:
        ser.write(b'P')  # Send 'P' command to trigger pump control
        time.sleep(5)  # Wait for Arduino to process and respond

        # Read and log Arduino responses
        while ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            log_message(line, pump_log_file_path)
            print(line)
        log_message("Pump control command executed successfully.", pump_log_file_path)
        
    except Exception as e:
        log_message(f"Error: {str(e)}", pump_log_file_path)

# MAIN EXECUTION
try:
    # Check the current time
    current_time = datetime.now()
    minute = current_time.minute

    # Perform sensor data upload every time script is run (every 30 minutes)
    sensor_data = fetch_sensor_data()
    upload_water_data(sensor_data)

    # Perform pump control at the top of every hour (every other script run)
    if minute == 0:
        time.sleep(180)  # Delay 3 minutes to allow for sensor reading and uploading (will overload wired connection)
        trigger_pump_control()

except Exception as e:
    log_message(f"Error: {str(e)}", sensor_log_file_path)