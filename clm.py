import socket
import time

def toFixed(numObj, digits=0):
    return f"{numObj:.{digits}f}"

numOfSec = 10000
print('Начало выполнения клиента...')
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('192.168.1.218', 5000)
print(f'Подключение к {server_address[0]} на порт {server_address[1]}')
sock.connect(server_address)

try:
    countBytes = 0                      # Количество байт за текущую секунду
    total_bytes = 0                     # Общее количество принятых байт
    start_time = time.time()            # Начальное время для общего расчёта
    interval_start_time = start_time    # Начало текущего интервала
    n = 0
    with open('client_output.txt', 'w') as f:
        f.write("sec,d1,d2,d3,d4,d5,d6,d7,d8")
    
        while n < numOfSec:
            # Получение данных от сервера
            data = sock.recv(16384)
            if not data:
                print("Соединение закрыто сервером.")
                break

            temp = 0
            number = 0
            while temp < len(data):
                if temp + 1 >= len(data):
                    break
                res = data[temp]
                #print(f"Received value: {res}")
                if n < numOfSec:
                    if number % 8 == 0:
                        f.write("\r\n")
                        f.write(str(n))
                        f.write(",")
                    else:
                        f.write(",")
                    f.write(str(res))
                temp += 1
                number += 1

            countBytes += len(data)
            total_bytes += len(data)

            current_time = time.time()
            elapsed_interval = current_time - interval_start_time

            if elapsed_interval >= 1.0:
                # Текущее количество бит и байт за прошедшую секунду
                current_bits_per_sec = countBytes * 8
                current_bytes_per_sec = countBytes
                current_khz = toFixed(current_bytes_per_sec / 16 / 1000, 1)
                print(f"{current_bits_per_sec} bits/s, {current_bytes_per_sec} bytes/s, {current_khz} kHz")
                # Сброс счётчиков для следующего интервала
                countBytes = 0
                n += 1
                interval_start_time = current_time

    total_time = time.time() - start_time
    if total_time > 0:
        average_mbit_s = (total_bytes * 8) / (total_time * 1e6)  # Мбит/с
    else:
        average_mbit_s = 0.0
    print(f"\nИтоговая средняя скорость передачи данных: {average_mbit_s:.2f} Мбит/с")

finally:
    print('Закрытие подключения')
    sock.close()
