# ultrasonic_distance_meter
An ultrasonic distance meter 

============
Dependencies
============

Install GCC for AVR and libraries
```
$ sudo apt-get install gcc-avr binutils-avr avr-libc avrdude
```
GDB for AVR
```
$ sudo apt-get install gdb-avr
```

Install Eclipse and the eclipse C development toolkit

```
apt-get install eclipse eclipse-cdt
```

Install eclipse avr plugin

Go to the Software installation dialog (Help > Install New Software...)

Then enter the updatesite URL http://avr-eclipse.sourceforge.net/updatesite into the "Work with:" field. Select the AVR Eclipse Plugin from the list, hit "Next>" and follow the instructions on the next pages.

AVREclipseInstallUpdatesiteScreenshot.png


===================
Configure Eclipse
===================

Go over Window > Preferences

Click on AVR > AVRDude tab
Click on Add button

Choose Programmer Hardware = Atmel AVR ISP

===================
Usage
===================
