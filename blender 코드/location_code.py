import serial
import time
import bpy
import threding

PORT='COM5'
BaudRate=115200
accel_const = 50000
current_v = [0.0, 0.0, 0.0]
current = [0.0, 0.0, 0.0]
cube = bpy.context.selected_objects[0]

def init_location_cursor():
    bpy.context.scene.cursor.location[0] = 0
    bpy.context.scene.cursor.location[1] = 0
    bpy.context.scene.cursor.location[2] = 0
    
def init_location_cube():
    cube.location[0] = 0
    cube.location[1] = 0
    cube.location[2] = 0

def get_accel():
    ser.write(b'get\n')
    received_data = str(ser.readline().decode())
    x_accel_value, y_accel_value, z_accel_value, x_rotation_value, y_rotation_value, z_rotation_value = received_data.split(',')
    accel_data = [float(x_accel_value), float(y_accel_value), float(z_accel_value)]
    accel_data = [accel_data[0]/accel_const, accel_data[1]/accel_const, accel_data[2]/accel_const]
    return accel_data
    
def init_setup():
    init_location_cursor()
    init_location_cube()
    

def locate_cube():
    for i in range(200):
        accel = get_accel()
        for j in range(len(current)):
            if abs(accel[j])>0.005:
                current_v[j] = current_v[j] + accel[j]
                current[j] = current_v[j] + current[j]
        cube.location = current
        print(current)
        bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP', iterations=1)

def end_serial():
    ser.write(b'stop\n')
    ser.close()
    

ser = serial.Serial(PORT, BaudRate)
time.sleep(3)

init_setup()
t = threading.Thread(target = locate_cube(), daemon = True)
t.daemon = True
t.start

end_serial()