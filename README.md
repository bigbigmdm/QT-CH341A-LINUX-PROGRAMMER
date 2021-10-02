# QT-CH341A-LINUX-PROGRAMMER
The Prog24 is a free I2C EEPROM programmer tools. The program use the CH341A programmer device.
This program uses QhexEditor (autor: Qingfeng Xia) from https://github.com/qingfengxia/qhexedit
and ch341 c-programmer tools  (autor: Collin Allen) from https://github.com/command-tab/ch341eeprom

I have modified the GUI in QhexEditor. I added buttons to read from eeprom, save to eeprom and programmer information menu.

![CH341A EEPROM programmer](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/prog24.gif)
 
 Easy steps to use:
 1. Insert the EEPROM 24Cxx chip in the correct place in the socket of the programmer. See the picture on the PCB of the programmer for information.
 2. Connent your CH341A Programmer device into usb port.
 ![CH341A programmer device](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/ch341_to_form_150_150.png)

3. Select the EEPROM chip in menu - 24C01, 24C02 ...24C512 (Press button ![select](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/chip_type.png)  ).
4. For reading from a chip select the 'Read from EEPROM' item. (Press button ![read](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/read.png)  ).
5. For saving the dump press the diskette button  ![save](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/save.png)  and setting the name of file.
6. For open the existing file press the folder icon  ![load](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/open.png)   and select the file.
7. For writing the dump to EEPROM press the 'Wirte to EEPROM'   ![write](https://github.com/bigbigmdm/QT-CH341A-LINUX-PROGRAMMER/raw/main/screenshot/write.png)   buttom.
