import socket
import threading
import json
import re
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 伺服器參數
HOST = '0.0.0.0'
PORT = 8080

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))
server_socket.listen(5)
print(f"伺服器已啟動，監聽 {HOST}:{PORT}")

# 建立數據存儲變數
x_data = []
y_data_accel_x, y_data_accel_y, y_data_accel_z = [], [], []
y_data_magneto_x, y_data_magneto_y, y_data_magneto_z = [], [], []
y_data_temp, y_data_humidity, y_data_distance = [], [], []

lock = threading.Lock()

# 創建 2x2 的圖表
fig, axes = plt.subplots(2, 2, figsize=(10, 8))
ax_accel, ax_magneto, ax_temp, ax_tof = axes.flatten()  # 轉成一維陣列

# 加速度計 (Accelerometer)
line_accel_x, = ax_accel.plot([], [], 'r-', label="Accel X")
line_accel_y, = ax_accel.plot([], [], 'g-', label="Accel Y")
line_accel_z, = ax_accel.plot([], [], 'b-', label="Accel Z")
ax_accel.set_xlim(0, 100)
ax_accel.set_ylim(-2000, 2000)
ax_accel.legend()
ax_accel.set_title("Accelerometer")

# 磁力計 (Magnetometer)
line_magneto_x, = ax_magneto.plot([], [], 'r-', label="Magneto X")
line_magneto_y, = ax_magneto.plot([], [], 'g-', label="Magneto Y")
line_magneto_z, = ax_magneto.plot([], [], 'b-', label="Magneto Z")
ax_magneto.set_xlim(0, 100)
ax_magneto.set_ylim(-2000, 2000)
ax_magneto.legend()
ax_magneto.set_title("Magnetometer")

# 溫度 (Temperature)
line_temp, = ax_temp.plot([], [], 'r-', label="Temperature")
ax_temp.set_xlim(0, 100)
ax_temp.set_ylim(40, 10)  # 讓溫度軸 40~10
ax_temp.invert_yaxis()
ax_temp.legend()
ax_temp.set_title("Temperature")

# Time of Flight
line_tof, = ax_tof.plot([], [], 'b-', label="Distance (mm)")
ax_tof.set_xlim(0, 100)
ax_tof.set_ylim(0, 5000)
ax_tof.legend()
ax_tof.set_title("Time of Flight")

def extract_json(data):
    match = re.search(r'\{.*\}', data, re.DOTALL)
    if match:
        return match.group(0)
    return None

def handle_client(conn, addr):
    print(f"[{addr}] 連線成功")

    while True:
        try:
            data = conn.recv(4096).decode('utf-8').strip()
            if not data:
                break

            json_str = extract_json(data)
            if not json_str:
                print(f"[{addr}] 無法解析 JSON 數據")
                continue

            sensor_data = json.loads(json_str)
            print(f"[{addr}] 解析成功: {sensor_data}")

            accel_x = accel_y = accel_z = 0
            magneto_x = magneto_y = magneto_z = 0
            temperature = humidity = distance = 0

            for sensor in sensor_data.get("iot2tangle", []):
                if sensor["sensor"] == "Accelerometer":
                    accel_x = float(sensor["data"][0]["x"])
                    accel_y = float(sensor["data"][0]["y"])
                    accel_z = float(sensor["data"][0]["z"])
                elif sensor["sensor"] == "Magnetometer":
                    magneto_x = float(sensor["data"][0]["x"])
                    magneto_y = float(sensor["data"][0]["y"])
                    magneto_z = float(sensor["data"][0]["z"])
                elif sensor["sensor"] == "Environmental":
                    humidity = float(sensor["data"][0]["Humidity"])
                    temperature = float(sensor["data"][0]["Temperature"])
                elif sensor["sensor"] == "Time of flight":
                    distance = float(sensor["data"][0]["distance"])

            with lock:
                x_data.append(len(x_data))
                y_data_accel_x.append(accel_x)
                y_data_accel_y.append(accel_y)
                y_data_accel_z.append(accel_z)

                y_data_magneto_x.append(magneto_x)
                y_data_magneto_y.append(magneto_y)
                y_data_magneto_z.append(magneto_z)

                y_data_temp.append(temperature)
                y_data_humidity.append(humidity)
                y_data_distance.append(distance)

                if len(x_data) > 100:
                    x_data.pop(0)
                    y_data_accel_x.pop(0)
                    y_data_accel_y.pop(0)
                    y_data_accel_z.pop(0)

                    y_data_magneto_x.pop(0)
                    y_data_magneto_y.pop(0)
                    y_data_magneto_z.pop(0)

                    y_data_temp.pop(0)
                    y_data_humidity.pop(0)
                    y_data_distance.pop(0)

        except (ValueError, json.JSONDecodeError, ConnectionResetError):
            print(f"[{addr}] 連線錯誤")
            break

    conn.close()
    print(f"[{addr}] 連線已關閉")

def accept_clients():
    while True:
        conn, addr = server_socket.accept()
        thread = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
        thread.start()

threading.Thread(target=accept_clients, daemon=True).start()

def update(frame):
    with lock:
        if not x_data:
            return line_accel_x, line_accel_y, line_accel_z, \
                   line_magneto_x, line_magneto_y, line_magneto_z, \
                   line_temp, line_tof
        
        # 更新 X 軸範圍
        ax_accel.set_xlim(x_data[0], x_data[-1])
        ax_magneto.set_xlim(x_data[0], x_data[-1])
        ax_temp.set_xlim(x_data[0], x_data[-1])
        ax_tof.set_xlim(x_data[0], x_data[-1])

        # 更新數據
        line_accel_x.set_data(x_data, y_data_accel_x)
        line_accel_y.set_data(x_data, y_data_accel_y)
        line_accel_z.set_data(x_data, y_data_accel_z)

        line_magneto_x.set_data(x_data, y_data_magneto_x)
        line_magneto_y.set_data(x_data, y_data_magneto_y)
        line_magneto_z.set_data(x_data, y_data_magneto_z)

        line_temp.set_data(x_data, y_data_temp)
        line_tof.set_data(x_data, y_data_distance)

    return line_accel_x, line_accel_y, line_accel_z, \
           line_magneto_x, line_magneto_y, line_magneto_z, \
           line_temp, line_tof

ani = FuncAnimation(fig, update, blit=False)

plt.tight_layout()
plt.show()

server_socket.close()
