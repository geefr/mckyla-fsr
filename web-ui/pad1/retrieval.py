#!/usr/bin/env python3

import cgi
import serial
import time

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


print("Content-type: text/html")
print()
print('''<html>''')

print('''<head>''')
print('''<link rel="stylesheet" type="text/css" href="../styles/styles.css">''')
print('''<script src="../js/jquery-3.2.1.min.js"></script>''')
print('''<script src="../js/scripts.js"></script>''')
print('''</head>''')

print('''<body>''')
print()

form = cgi.FieldStorage()
cur_user = form.getvalue("cur_user")
l_pressure = form.getvalue("left_pressure")
u_pressure = form.getvalue("up_pressure")
r_pressure = form.getvalue("right_pressure")
d_pressure = form.getvalue("down_pressure")
autocalibrate_threshold = form.getvalue("autocalibrate_threshold")
s = getSerialConnection("left")
s.setDTR(1)
f = open("users.txt", "rb")
users_file = f.read().decode('utf-8')
f.close()
user_list = users_file.split("^")
cur_user_list_index = -1
for u in range(len(user_list)):
    if user_list[u].split(":")[0] == cur_user:
        cur_user_list_index = u
        break
cur_user_list = user_list[cur_user_list_index].strip("\n").split(":")

if autocalibrate_threshold:
    s.write(("C"+autocalibrate_threshold+"\r\n").encode('utf-8'))
    new_pressures = s.read(78).decode('utf-8')
else:
    if (len(l_pressure) == 3):
        if cur_user_list_index != -1:
            cur_user_list[1]=l_pressure
        s.write(("0"+l_pressure+"\r\n").encode('utf-8'))
        new_pressures = s.read(78).decode('utf-8')
    if (len(u_pressure) == 3):
        if cur_user_list_index != -1:
            cur_user_list[2]=u_pressure
        s.write(("1"+u_pressure+"\r\n").encode('utf-8'))
        new_pressures = s.read(78).decode('utf-8')
    if (len(r_pressure) == 3):
        if cur_user_list_index != -1:
            cur_user_list[3]=r_pressure
        s.write(("2"+r_pressure+"\r\n").encode('utf-8'))
        new_pressures = s.read(78).decode('utf-8')
    if (len(d_pressure) == 3):
        if cur_user_list_index != -1:
            cur_user_list[4]=d_pressure
        s.write(("3"+d_pressure+"\r\n").encode('utf-8'))
        new_pressures = s.read(78).decode('utf-8')
    if (len(l_pressure) != 3 and len(u_pressure)  != 3 and len(r_pressure)  != 3 and len(d_pressure)  != 3):
        s.write("7\r\n".encode('utf-8'))
        new_pressures = s.read(78).decode('utf-8')
print(new_pressures.replace(",", "|"))
print('''<br><a href=pads.py?cur_user=%s>Return</a>''' % cur_user)

user_list[cur_user_list_index] = ":".join(cur_user_list)
f = open("users.txt", "wb")
f.write(("^".join(user_list)).encode('utf-8'))
f.close

s.close()
print('''<script>setTimeout(function() { window.location = "pads.py?cur_user=%s" }, 1000) </script>''' % cur_user)

print('''</body>''')
print('''</html>''')
