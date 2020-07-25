#!/usr/bin/env python3

import cgi
import serial

def getSerialConnection(padSideByteString):
	padSideStr = "0".encode('utf-8') if (padSideByteString == "left") else "1".encode('utf-8')

	s = serial.Serial("/dev/ttyACM0", 9600)
	s.setDTR(1)

	#Send 9: Gief pad side from ttyACM0
	s.write("9\r\n".encode('utf-8'))
	padSide = s.readline()

	if padSide[0] != padSideStr[0]:
		#Turns out he was the other side, so ttyACM1 has our pad! We connect to him now!
		s.close()

		s = serial.Serial("/dev/ttyACM1", 9600)
		s.setDTR(1)
	
	return s

s = getSerialConnection("left")
s.write("E\r\n".encode('utf-8'))
s.close()

form = cgi.FieldStorage()
cur_user = form.getvalue("cur_user")

print("Content-type: text/html")
print()
print('''<html>''')

print('''<head>''')
print('''<link rel="stylesheet" type="text/css" href="../styles/styles.css">''')
print('''<script src="../js/jquery-3.2.1.min.js"></script>''')
print('''<script src="../js/scripts.js"></script>''')
print('''</head>''')

print('''<body>''')
print('''<div class="button" style="padding: 10px; margin: 0 auto;margin-top: 25px;width: 300px;">''')
print('''    <a href="vdcm-disable.py?cur_user=%s" style="text-decoration: none;">Stop pad vibration detection calibration</a>''' % cur_user)
print('''</div>''')

print('''</body>''')
print('''</html>''')
