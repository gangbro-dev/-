#import bpy
import threading
import time
#import serial

#PORT = 'COM3'
#BaudRate = 115200
Lock = threading.Lock()
rot_data = list()
#Quat = serial.Serial(PORT, BaudRate)

#def init_rotation():
#    bpy.context.scene.cursor.rotation_mode = 'QUATERNION'
#    bpy.context.scene.cursor.rotation_quaternion[0] = 1
#    bpy.context.scene.cursor.rotation_quaternion[1] = 0
#    bpy.context.scene.cursor.rotation_quaternion[2] = 0
#    bpy.context.scene.cursor.rotation_quaternion[3] = 0


def getrotation():
    global rot_data
#    global Quat
    while True:
#        received_data = str(Quat.readline().decode()[:len(Quat.readline()) - 2])
        w_rotation_value, x_rotation_value, y_rotation_value, z_rotation_value = ["1", "0", "0", "0"]
        rot_data.append([float(w_rotation_value), float(x_rotation_value), float(y_rotation_value), float(z_rotation_value)])
        print("A routine\n")
        if len(rot_data) == 1:
            break



def remote_cursor():
    global rot_data
    while True:
        rotation_data = rot_data.pop()
        print("B routine\n")
        if len(rot_data) == 0:
            break


def CursorRotation():
    i = 0
    while True:
        if i>100:
            break
        i += 1
        getrotation()
        remote_cursor()
        time.sleep(0.1)


Rot_Thread = threading.Thread(target=CursorRotation(), args=())
Rot_Thread().join



