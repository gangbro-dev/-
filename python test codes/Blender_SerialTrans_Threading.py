import bpy
import threading
import serial
import time
import locale

PORT = 'COM3'
BaudRate = 115200
rot_data = list()
Quat = serial.Serial(PORT, BaudRate)


def init_rotation():
    bpy.context.scene.cursor.rotation_mode = 'QUATERNION'
    bpy.context.scene.cursor.rotation_quaternion[0] = 1
    bpy.context.scene.cursor.rotation_quaternion[1] = 0
    bpy.context.scene.cursor.rotation_quaternion[2] = 0
    bpy.context.scene.cursor.rotation_quaternion[3] = 0


def get_rotation():
    global rot_data
    global Quat
    received_data = str(Quat.readline().decode()[:len(Quat.readline()) - 2])
    w_rotation_value, x_rotation_value, y_rotation_value, z_rotation_value = received_data.split(',')
    w_rot = locale.atof(w_rotation_value)
    x_rot = locale.atof(x_rotation_value)
    y_rot = locale.atof(y_rotation_value)
    z_rot = locale.atof(z_rotation_value)
    rot_data.append([w_rot, x_rot, y_rot, z_rot])


def remote_cursor():
    global rot_data
    if len(rot_data) >= 1:
        bpy.context.scene.cursor.rotation_quaternion = rot_data.pop()


def cursor_rotation():
    i = 0
    while True:
        if i > 100:
            break
        i += 1
        print("before get_rotation")
        get_rotation()
        time.sleep(0.5)
        print("before remote_cursor")
        remote_cursor()
        time.sleep(0.5)


init_rotation()

remote_cursorThread = threading.Thread(target=cursor_rotation(), args=(), daemon=True)
