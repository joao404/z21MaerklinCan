# z21MaerklinCan

This project is based on the work of:
- Philipp Gathow (https://pgahtow.de/w/Z21_mobile)
- Joerg Pleumann (https://github.com/MBuratto/railuino)
- Gerhard Bertelsmann (https://github.com/GBert/railroad/blob/master/can2udp/src/can2lan.c)
- Hardi-St(https://github.com/Hardi-St/MirZ21)

Provides z21 and RemoteCS2/3 app(can2lan) server for Maerklin Trainbox 60113. Needs an Esp32 and a Tja1050.

Programming of locos through z21 app is deactivated by default and can be activated in webserver(will automatic be deactivated by next reboot).

Known problems:
- z21 loco programming is not tested
- Reading of configuration from additional MS2 with software version higher then 3.0 reads only locos
- z21 app does not get stop or go command from MS. Reason unknown

ToDo:
- Configuration Writing for z21
- Reading of configuration from additional MS2 with software version less than 3.0

Wich:
- Reading of Mfx loco to make MS2 redundant => loco managment needed in that case.
  Possible solution is output of mfx loco address on webserver for z21 app

Note:
- The Z21 interface only notifies loco values, if this loco was controlled before

## Programming
- Use PlatformIO to build Filesystem Image
- Upload FilesystemImage
- Upload code
## Usage
- Connect to new access point with password 12345678
- Start RemoteCS2/3 or z21 App. Connect to server under ip 192.168.4.1

## Loading locos from MS2
- Go to webserver under 192.168.4.1
- Go to Home
- mark checkbox for "Read locos from Mobile Station"
- Click on Submit
- There is currently no feedback when transmission finished
