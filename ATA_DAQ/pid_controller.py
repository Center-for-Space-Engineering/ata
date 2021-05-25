import time
import pandas as pd
from simple_pid import PID

right_pid = PID()
right_pid.setpoint = 105
right_pid.output_limits = (1, 15) 

right_pid.tunings = (1.0, 0.0, 0.0)

while(1):
	try:
		data = pd.read_csv('current_temp.csv')
		right = data['Right']
		
		right_num = float(right)
		
		control = right_pid(right_num)
			
		print(control)
		
		temp = control*1024.0
		hex_control = hex(int(temp))
		
		commands = open("rt_commands.csv", "w");
		commands.write("0x4E, WB, 0x00, 0x00,\n");
		commands.write("0x5B, WB, 0x01, 0x80,\n");
		commands.write("0x4E, WW, 0x21," + hex_control + ",");
		commands.close()
		time.sleep(8)
	except:
		time.sleep(0.25)
	
	
	
