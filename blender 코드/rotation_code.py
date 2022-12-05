import serial
import time
import bpy

PORT='COM5'
BaudRate=115200

cube = bpy.context.selected_objects[0]

def init_rotation_cursor():
    bpy.context.scene.cursor.rotation_mode = 'XYZ'
    bpy.context.scene.cursor.rotation_euler[0] = 0
    bpy.context.scene.cursor.rotation_euler[1] = 0
    bpy.context.scene.cursor.rotation_euler[2] = 0
    
def init_rotation_cube():
    cube.rotation_euler[0] = 0
    cube.rotation_euler[0] = 0
    cube.rotation_euler[0] = 0

def get_rotation():
    ser.write(b'geta\n')
    received_data = str(ser.readline().decode())
    x_location_value, y_location_value, z_location_value,x_rotation_value, y_rotation_value, z_rotation_value = received_data.split(',')
    rot_data = [float(x_rotation_value), float(y_rotation_value), float(z_rotation_value)]
    rot_data = [rot_data[0], rot_data[1], rot_data[2]]
    return rot_data

init_rotation_cursor()
init_rotation_cube()
current = [0.0, 0.0, 0.0]
ser = serial.Serial(PORT, BaudRate)
time.sleep(3)
for x in range(500):
    current = [current[i] + get_rotation()[i] for i in range(len(current))]
    cube.rotation_euler = current
    bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP', iterations=1)
    
ser.write(b'stop\n')
ser.close();