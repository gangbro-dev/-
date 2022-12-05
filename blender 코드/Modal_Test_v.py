import serial
import time
import bpy
import threading

PORT='COM5'
BaudRate=115200
accel_const = 50000
vel_const = 50
loc_const = 100
current_a = [0.0, 0.0, 0.0]
current_v = [0.0, 0.0, 0.0]
current = [0.0, 0.0, 0.0]
cube = bpy.context.selected_objects[0]
m = [[0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [0.0, 0.0, 0.0]]

t1 = time.time()
t2 = time.time()

def get_accel():
    ser.write(b'geta\n')
    received_data = str(ser.readline().decode())
    x_accel_value, y_accel_value, z_accel_value, x_rotation_value, y_rotation_value, z_rotation_value = received_data.split(',')
    accel_data = [float(x_accel_value), float(y_accel_value), float(z_accel_value)]
    accel_data = [accel_data[0]/accel_const, accel_data[1]/accel_const, accel_data[2]/accel_const]
    return accel_data

def get_vel():
    ser.write(b'getv\n')
    received_data = str(ser.readline().decode())
    x_vel_value, y_vel_value, z_vel_value = received_data.split(',')
    vel_data = [float(x_vel_value), float(y_vel_value), float(z_vel_value)]
    vel_data = [vel_data[0]/vel_const, vel_data[1]/vel_const, vel_data[2]/vel_const]
    return vel_data

def get_loc():
    ser.write(b'getl\n')
    received_data = str(ser.readline().decode())
    x_loc_value, y_loc_value, z_loc_value = received_data.split(',')
    loc_data = [float(x_loc_value), float(y_loc_value), float(z_loc_value)]
    loc_data = [loc_data[0]/loc_const, loc_data[1]/loc_const, loc_data[2]/loc_const]
    return loc_data

class ModalOperator(bpy.types.Operator):
    
    bl_idname = "object.modal_operator"
    bl_label = "Simple Modal Operator"
    
    def __init__(self):
        ser.write(b'set\n')
        print("Start")
    
    def __del__(self):
        ser.write(b'stop\n')
        ser.close()
        print("End")
    
    def execute(self, context):
        context.object.location.x = self.value_x 
        context.object.location.y = self.value_y
        context.object.location.z = self.value_z
        return {'FINISHED'}
    def modal(self, context, event):
        global t1, t2, m
        sum = [0, 0, 0]
        window_size = 3
        current_v = get_vel()
        
        t2 = t1
        t1 = time.time()
        dt = t2 - t1
        
#        print(dt)
#        m[0] = m[1]
#        m[1] = m[2]
#        m[2] = current_v
#        for i in range(window_size):
#            sum[i] = m[0][i] + m[1][i] + m[2][i]
#            current_v[i] = sum[i]/window_size
        for i in range(len(current)):
                current[i] = current[i] + (current_v[i] * dt)
        self.value_x = current[0]
        self.value_y = current[1]
        self.value_z = current[2]
        print(current)
        self.execute(context)
        if event.type == 'LEFTMOUSE':
            return {'FINISHED'}
        elif event.type in {'RIGHTMOUSE', 'ESC'}:
            context.object.location.x = self.init_loc_x
            context.object.location.y = self.init_loc_y
            context.object.location.z = self.init_loc_z
            return {'CANCELLED'}
        
        return {'RUNNING_MODAL'}
    
    def invoke(self, context, event):
        cube.location.x = 0
        cube.location.y = 0
        cube.location.z = 0
        self.init_loc_x = context.object.location.x
        self.init_loc_y = context.object.location.y
        self.init_loc_z = context.object.location.z
        self.value_x = 0
        self.value_y = 0
        self.value_z = 0
        self.execute(context)
        
        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}
    

ser = serial.Serial(PORT, BaudRate)
time.sleep(3)

bpy.utils.register_class(ModalOperator)

# test call
#bpy.ops.object.modal_operator.

bpy.ops.object.modal_operator('INVOKE_DEFAULT')
