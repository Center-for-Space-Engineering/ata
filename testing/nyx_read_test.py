from pyBusPirateLite.pyBusPirateLite.SPI import SPI
import time
import csv
from datetime import datetime

"""
Example code from library

from pyBusPirateLite.SPI import SPI
spi = SPI()
spi.pins = SPI.PIN_POWER | SPI.PIN_CS
spi.config = SPI.CFG_PUSH_PULL | SPI.CFG_IDLE
spi.speed = '1MHz'
spi.cs = True
data = spi.transfer( [0x82, 0x00])
spi.cs = False
"""

# Turn on V_bias and clear unwanted things
def configure(spi, byte=0xC3):
	spi.config = SPI.CFG_PUSH_PULL | SPI.CFG_IDLE
	spi.speed = '1MHz'
	
	set_pins(spi, True)
	spi.write_then_read(2, 0, [0x80, byte])
	
	set_pins(spi, False)
	spi.write_then_read(2, 0, [0x80, byte])

def read_from_sensor(spi):
	# Read data. This function is used because `transfer` does not behave
	# as expected
	msb = int.from_bytes(spi.write_then_read(1,1,[0x01]), "big")
	lsb = int.from_bytes(spi.write_then_read(1,1,[0x02]), "big")
	
	# Concatenate data
	data = (msb << 7 & 0x7f80)  + (lsb >> 1 & 0x7f)
	
	return data

def read_all_registers(spi):
	bytes_to_send = [0x00, 0x01, 0x02, 0x03,
					 0x04, 0x05, 0x06, 0x07,
					 0x08, 0x09, 0x0A, 0x0B,
					 0x0C, 0x0D, 0x0E, 0x0F]
	
	for i in range(8):
		data = spi.write_then_read(1,1,[bytes_to_send[i]])
		print(f"Byte {i}: {data}")

# Toggle which output is being used
def set_pins(spi, aux):
	if aux:
		spi.pins = SPI.PIN_POWER | SPI.PIN_CS
	else:
		spi.pins = SPI.PIN_POWER | SPI.PIN_CS | SPI.PIN_AUX

# Read from both sensors and return the values read
# `lines` specifies number of sensors attached to the board
def read(spi, lines=2):
	if (lines == 2):
		# Set pins to read from SPI0
		set_pins(spi, True)
	
	val1 = read_from_sensor(spi)
	
	if (lines == 2):
		# Set pins to read from SPI0
		set_pins(spi, False)
		val2 = read_from_sensor(spi)
	else:
		val2 = -1
	
	# Sanitize if open circuit
	if val1 == 32767:
		val = "NaN"
	if val2 == 32676:
		val = "NaN"
		
	return (val1, val2)

# Create the object
spi1 = SPI(portname='/dev/ttyUSB0')
spi2 = SPI(portname='/dev/ttyUSB1')

# Set up the sensor
configure(spi1)
configure(spi2)

# File to log data to
with open("SPI_logging.csv",'w',buffering=1) as csvfile:
	#writer = csv.DictWriter(csvfile, fieldnames=[""]
#csvfile = open("SPI_logging.csv",'w')

	writer = csv.writer(csvfile)
	header = ["Sample","Time","SPI0","SPI1","SPI2","SPI3"]
	writer.writerow(header)

	count = 0
	#while count < 100:
	while True:
		t = datetime.now()
		(data1, data2) = read(spi1, lines=2)
		(data3, data4) = read(spi2, lines=2)
		#(data3, data4) = (0,0)
		#print(count,"seconds:",data1,data2,data3,data4)
		currentTime = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
		print(f"{currentTime}, running for {count} seconds: {data1}, {data2}, {data3}, {data4}")
		count += 1
		
		# Add to file
		writer.writerow([count, t.strftime("%H:%M:%S"), data1, data2, data3, data4])
		
		time.sleep(1)

#csvfile.close()
