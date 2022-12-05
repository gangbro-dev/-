import serial
import time
import bpy
import threading

PORT='COM5'
BaudRate=115200
accel_const = 50000
current_v = [0.0, 0.0, 0.0]
current = [0.0, 0.0, 0.0]
cube = bpy.context.selected_objects[0]
m = [0.0, 0.0, 0.0]

t1 = time.time()
t2 = time.time()

def get_accel():
    ser.write(b'get\n')
    received_data = str(ser.readline().decode())
    x_accel_value, y_accel_value, z_accel_value, x_rotation_value, y_rotation_value, z_rotation_value = received_data.split(',')
    accel_data = [float(x_accel_value), float(y_accel_value), float(z_accel_value)]
    accel_data = [accel_data[0]/accel_const, accel_data[1]/accel_const, accel_data[2]/accel_const]
    return accel_data


class ModalOperator(bpy.types.Operator):
    
    bl_idname = "object.modal_operator"
    bl_label = "Simple Modal Operator"
    
    def __init__(self):
        print("Start")
    
    def __del__(self):
        ser.write(b'stop\n')
        ser.close()
        print("End")
    
    def execute(self, context):
        context.object.location.x = self.value_x 
        context.object.location.y = self.value_y
        return {'FINISHED'}
    
    def modal(self, context, event):
        global t1
        global t2
        global m
        window_size = 3
        accel = get_accel()
        t2 = time.time()
        dt = t2 - t1
        t1 = time.time()
        m[0] = m[1]
        m[1] = m[2]
        m[2] = accel
        sum = sum + m[0] + m[1] + m[2]
        accel = sum / window_size
        
        
        for i in range(len(current)):
            if abs(accel[j])>0.002:
                current_v[j] = current_v[j] + (accel[j] *dt)
                current[j] = current_v[j] + (current[j] *dt)
        self.value_x = current[0]
        self.value_y = current[1]
        print(current)
        self.execute(context)
        if event.type == 'LEFTMOUSE':
            return {'FINISHED'}
        elif event.type in {'RIGHTMOUSE', 'ESC'}:
            context.object.location.x = self.init_loc_x
            context.object.location.y = self.init_loc_y
            return {'CANCELLED'}
        
        return {'RUNNING_MODAL'}
    
    def invoke(self, context, event):
        cube.location.x = 0
        cube.location.y = 0
        cube.location.z = 0
        self.init_loc_x = context.object.location.x
        self.init_loc_y = context.object.location.y
        self.value_x = 0
        self.value_y = 0
        self.execute(context)
        
        context.window_manager.modal_handler_add(self)
        return {'RUNNING_MODAL'}
    

ser = serial.Serial(PORT, BaudRate)
time.sleep(3)

bpy.utils.register_class(ModalOperator)

# test call
#bpy.ops.object.modal_operator.

bpy.ops.object.modal_operator('INVOKE_DEFAULT')
