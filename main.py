import serial
import time

# ⚠️ 1. 把 COM9 换成你设备管理器里 USB转TTL 的 COM口
# ⚠️ 2. 确保 STM32 已经通电，且 USB转TTL 模块插在电脑上
COM_PORT = 'COM9'
BAUD_RATE = 115200

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"✅ 串口 {COM_PORT} 已打开，等待接收STM32数据...")
    print("（如果没反应，请检查接线：TTL模块的TX接PA10，RX接PA9，GND接GND）")

    while True:
        if ser.in_waiting > 0:
            # 读取STM32发来的一行数据
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"收到: {line}")
except Exception as e:
    print(f"❌ 串口打开失败: {e}")
    print("请检查：1. COM号填对了吗？ 2. 串口是否被其他软件占用？")