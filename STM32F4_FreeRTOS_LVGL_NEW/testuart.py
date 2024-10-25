import serial
import serial.tools.list_ports
import time

class Uart:
    def __init__(self):
        self.ser = None
        self.port = None
        self.baudrate = 9600
        self.timeout = 0.1
        self.connected = False

    def list_ports(self):
        ports = serial.tools.list_ports.comports()
        for port, desc, hwid in sorted(ports):
            print("{}: {} [{}]".format(port, desc, hwid))

    def connect(self, port):
        self.port = port
        self.ser = serial.Serial(port, self.baudrate, timeout=self.timeout)
        self.connected = True

    def disconnect(self):
        self.ser.close()
        self.connected = False

    def send(self, data):
        self.ser.write(data)

    def receive(self):
        return self.ser.read_all()

    def read(self, size):
        return self.ser.read(size)

    def write(self, data):
        self.ser.write(data)

    def readline(self):
        return self.ser.readline()

    def readlines(self):
        return self.ser.readlines()

    def flush(self):
        self.ser.flush()

    def reset_input_buffer(self):
        self.ser.reset_input_buffer()

    def reset_output_buffer(self):
        self.ser.reset_output_buffer()

    def in_waiting(self):
        return self.ser.in_waiting

    def out_waiting(self):
        return self.ser.out_waiting

    def close(self):
        self.ser.close()

    def __del__(self):
        if self.connected:
            self.close()

# Path: main.py
import time

uart = Uart()


filename = ""
last_time = ""
fist_data = ""
last_data = ""

uart_is_active      = False
uart_connected_flag = False

use_com = "";

while True:
    if uart_connected_flag == True:
        time.sleep(0.5)
        tt = uart.receive()
        if tt:
            
            tt = tt.decode("utf-8")
            #print(tt)
            if "start" in tt:
                fist_data = "开始时间：%s  EC值：%s" %(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), tt.split(" ")[3])
                print(fist_data)
            
            if "stop" in tt:
                last_time = time.strftime("%Y-%m-%d %H-%M-%S", time.localtime())
                last_data = "结束时间：%s  EC值：%s  TOC值：%s"%(last_time, tt.split(" ")[3], tt.split(" ")[5])
                print(last_data)
                filename = last_time + ".txt"
                with open(filename, "a",encoding="utf-8") as f:
                    f.write(fist_data + "\n")
                    f.write(last_data + "\n")
                    f.close()
    if uart_connected_flag == False:
        use_com = input("请输入串口号：")
        port_list = list(serial.tools.list_ports.comports())
        print(port_list)
        if len(port_list) > 0:
            for i in  list(range(len(port_list))):
                print(port_list[i])
                if "Silicon" in port_list[i][1]:
                    uart.connect(port_list[i][0])
                    uart_connected_flag = True
                    break