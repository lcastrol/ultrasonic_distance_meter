import io
import sys
import serial

ser = serial.Serial('/dev/tty.usbmodem1421', 9600) # Establish the connection on a specific port
counter=0 # Below 32 everything in ASCII is gibberish
while counter < 500:
	#ser.write(str(chr(counter))) # Convert the decimal number to ASCII then send it to the Arduino
	start_symbol = '';
	while start_symbol <> 'S':
		start_symbol = ser.read();

	a = ser.read(2) # Read two characters at once
	#a = ser.read()
	#print a
	temp = (ord(a[1])<<8) + ord(a[0]) # it is bigger by 1 check why
	#print temp
	#works fine, changing order might be necessary first byte more significant
	
	calibration = 3.9*4*4;
    #my $calibration = 0;
	ticks = temp; 
	if (ticks != 65535):
		cm = (ticks-calibration) * 64 * 331 / (16000 * 10);
		ms = ticks * 64 / (16000);
		#print "\033[2J";    #clear the screen
		#print "\033[0;0H"; #jump to 0,0 
		
		#print ms,;
		#print "ms";
		print int(cm);
		#print "cm";
		#print ticks;
		#print "ticks\n";
	else:
		print "OUT OFF LIMIT\n";


	counter = counter +1
sys.exit()