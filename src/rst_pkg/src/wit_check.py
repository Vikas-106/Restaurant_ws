#!/usr/bin/env python3
# -*- coding:utf-8 -*-
import serial
import struct
import rospy
import time
import math
import sys
import platform
import threading
import serial.tools.list_ports
from sensor_msgs.msg import Imu
from sensor_msgs.msg import MagneticField
from sensor_msgs.msg import NavSatFix
from std_msgs.msg import String
from tf.transformations import quaternion_from_euler

def find_ttyUSB():
    print('The default serial port of the imu is /dev/ttyACM1, if multiple serial port devices are identified, modify the serial port corresponding to the imu in the launch file')
    posts = [port.device for port in serial.tools.list_ports.comports() if 'ACM' in port.device]
    print('当前电脑所连接的 {} 串口设备共 {} 个: {}'.format('ACM', len(posts), posts))
    print('There are {} {} serial port devices connected to the current PC: {}'.format(len(posts), 'ACM', posts))

def checkSum(list_data, check_data):
    return sum(list_data) & 0xff == check_data

def handleSerialData(raw_data):
    global buff, key, angle_degree, magnetometer, acceleration, angularVelocity, pub_flag
    
    if python_version == '3':
        buff[key] = raw_data
    else:
        buff[key] = ord(raw_data)
        
    key += 1
    
    # Wait until we have a complete packet (20 bytes based on the data format)
    if key < 20:
        return
        
    # Verify packet header (0x55 0x61)
    if buff[0] != 0x55 or buff[1] != 0x61:
        key = 0
        buff = {}
        return
        
    try:
        data_buff = list(buff.values())
        
        # Extract acceleration data (bytes 2-7)
        acceleration = [
            struct.unpack('h', bytes([data_buff[2], data_buff[3]]))[0] / 32768.0 * 16 * 9.8,
            struct.unpack('h', bytes([data_buff[4], data_buff[5]]))[0] / 32768.0 * 16 * 9.8,
            struct.unpack('h', bytes([data_buff[6], data_buff[7]]))[0] / 32768.0 * 16 * 9.8
        ]
        
        # Extract angular velocity (bytes 8-13)
        angularVelocity = [
            struct.unpack('h', bytes([data_buff[8], data_buff[9]]))[0] / 32768.0 * 2000 * math.pi / 180,
            struct.unpack('h', bytes([data_buff[10], data_buff[11]]))[0] / 32768.0 * 2000 * math.pi / 180,
            struct.unpack('h', bytes([data_buff[12], data_buff[13]]))[0] / 32768.0 * 2000 * math.pi / 180
        ]
        
        # Extract angles (bytes 14-19)
        angle_degree = [
            struct.unpack('h', bytes([data_buff[14], data_buff[15]]))[0] / 32768.0 * 180,
            struct.unpack('h', bytes([data_buff[16], data_buff[17]]))[0] / 32768.0 * 180,
            struct.unpack('h', bytes([data_buff[18], data_buff[19]]))[0] / 32768.0 * 180
        ]
        
        # Publish the data
        stamp = rospy.get_rostime()
        
        imu_msg.header.stamp = stamp
        imu_msg.header.frame_id = "base_link"
        
        # Convert Euler angles to quaternion
        angle_radian = [math.radians(x) for x in angle_degree]
        qua = quaternion_from_euler(angle_radian[0], angle_radian[1], angle_radian[2])
        
        imu_msg.orientation.x = qua[0]
        imu_msg.orientation.y = qua[1]
        imu_msg.orientation.z = qua[2]
        imu_msg.orientation.w = qua[3]
        
        imu_msg.angular_velocity.x = angularVelocity[0]
        imu_msg.angular_velocity.y = angularVelocity[1]
        imu_msg.angular_velocity.z = angularVelocity[2]
        
        imu_msg.linear_acceleration.x = acceleration[0]
        imu_msg.linear_acceleration.y = acceleration[1]
        imu_msg.linear_acceleration.z = acceleration[2]
        
        # Debug print
        print(f"Publishing IMU data:")
        print(f"  Acceleration: {acceleration}")
        print(f"  Angular Velocity: {angularVelocity}")
        print(f"  Angles: {angle_degree}")
        
        imu_pub.publish(imu_msg)
        
    except Exception as e:
        print(f"Error processing packet: {e}")
    
    # Reset buffer
    key = 0
    buff = {}

def callback(data):
    global readreg, flag, calibuff, wt_imu, iapflag
    # Command definitions
    unlock_imu_cmd = b'\xff\xaa\x69\x88\xb5'
    enter_mag_cali_cmd = b'\xff\xaa\x01\x09\x00'
    exit_cali_cmd = b'\xff\xaa\x01\x00\x00'
    save_param_cmd = b'\xff\xaa\x00\x00\x00'
    
    print('Received command:', data.data)
    
    if "calibrate" in data.data.lower():
        wt_imu.write(unlock_imu_cmd)
        time.sleep(0.1)
        wt_imu.write(enter_mag_cali_cmd)
        print("Started calibration")
    elif "save" in data.data.lower():
        wt_imu.write(unlock_imu_cmd)
        time.sleep(0.1)
        wt_imu.write(exit_cali_cmd)
        time.sleep(0.1)
        wt_imu.write(save_param_cmd)
        print("Saved calibration")

def thread_job():
    print("ROS spin thread started")
    rospy.spin()

if __name__ == "__main__":
    # Initialize global variables
    key = 0
    flag = 0
    iapflag = 0
    buff = {}
    calibuff = []
    angularVelocity = [0, 0, 0]
    acceleration = [0, 0, 0]
    magnetometer = [0, 0, 0]
    angle_degree = [0, 0, 0]
    
    python_version = platform.python_version()[0]
    
    # Initialize ROS node
    find_ttyUSB()
    rospy.init_node("imu")
    port = rospy.get_param("~port", "/dev/ttyACM0")
    baudrate = rospy.get_param("~baud", 115200)
    print("IMU Type: Normal Port:%s baud:%d" % (port, baudrate))
    
    # Initialize ROS messages
    imu_msg = Imu()
    mag_msg = MagneticField()
    location_msg = NavSatFix()
    
    # Set up ROS subscriber for commands
    rospy.Subscriber("/wit/cmd", String, callback)
    
    # Start ROS spin thread
    add_thread = threading.Thread(target=thread_job)
    add_thread.start()
    
    # Initialize serial port
    try:
        wt_imu = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=0.5,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )
        
        if wt_imu.isOpen():
            rospy.loginfo("\033[32mSerial port opened successfully\033[0m")
        else:
            wt_imu.open()
            rospy.loginfo("\033[32mSerial port opened successfully\033[0m")
    except Exception as e:
        print(f"Error opening serial port: {e}")
        rospy.loginfo("\033[31mFailed to open serial port\033[0m")
        exit(0)
    
    # Set up ROS publishers
    imu_pub = rospy.Publisher("wit/imu", Imu, queue_size=10)
    mag_pub = rospy.Publisher("wit/mag", MagneticField, queue_size=10)
    location_pub = rospy.Publisher("wit/location", NavSatFix, queue_size=10)
    
    # Main loop
    while not rospy.is_shutdown():
        try:
            # Check for available data
            buff_count = wt_imu.inWaiting()
            if buff_count > 0 and iapflag == 0:
                buff_data = wt_imu.read(buff_count)
                print(f"Received data: {[hex(x) for x in buff_data]}")
                
                # Process each byte
                for byte_data in buff_data:
                    handleSerialData(byte_data)
                    
        except Exception as e:
            print(f"Error in main loop: {e}")
            print("IMU connection lost or wire broken")
            exit(0)

