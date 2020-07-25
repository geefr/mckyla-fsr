import serial

def getSerialConnection(padSideByteString):
	padSideStr = "0".encode('utf-8') if (padSideByteString == "left") else "1".encode('utf-8')

	s = serial.Serial("/dev/ttyACM0", 9600)
	s.setDTR(1)

	#Send 9: Gief pad side from ttyACM0
	s.write("9\r\n".encode('utf-8')
	padSide = s.readline().decode('utf-8')

	if padSide[0] != padSideStr[0]:
		#Turns out he was the other side, so ttyACM1 has our pad! We connect to him now!
		s.close()

		s = serial.Serial("/dev/ttyACM1", 9600)
		s.setDTR(1)
	
	return s

s = getSerialConnection("left")
s.write("C250\r\n".encode('utf-8')
s.close()
