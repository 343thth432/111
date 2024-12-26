import socket
import time

def measure_transfer_speed(server_ip, server_port, buffer_size=8192):
    # Создаем сокет
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        # Устанавливаем соединение с сервером
        sock.connect((server_ip, server_port))

        total_bytes_received = 0
        start_time = time.time()

        try:
            while True:
                # Получаем данные от сервера
                data = sock.recv(buffer_size)
                if not data:
                    break  # Выход, если данных больше нет

                total_bytes_received += len(data)

                # Проверяем, прошло ли 10 секунд
                elapsed_time = time.time() - start_time
                if elapsed_time >= 10:
                    # Рассчитываем скорость передачи данных
                    speed_bps = total_bytes_received / elapsed_time  # байт в секунду
                    speed_bps_bits = speed_bps * 8  # бит в секунду
                    speed_mbps = speed_bps_bits / (1000 * 1000)  # мегабит в секунду
                    print(f"Общий объем полученных данных: {total_bytes_received} байт")
                    print(f"Время измерения: {elapsed_time:.4f} секунд")
                    print(f"Средняя скорость передачи:")
                    print(f"  - {speed_bps:.2f} байт/сек")
                    print(f"  - {speed_bps_bits:.2f} бит/сек")
                    print(f"  - {speed_mbps:.2f} Мбит/сек")

                    # Сбрасываем счетчики для следующего периода
                    total_bytes_received = 0
                    start_time = time.time()

        except Exception as e:
            print(f"Произошла ошибка: {e}")

if __name__ == '__main__':
    # Адрес и порт вашего сервера
    SERVER_IP = '192.168.1.218'
    SERVER_PORT = 5000

    measure_transfer_speed(SERVER_IP, SERVER_PORT)


