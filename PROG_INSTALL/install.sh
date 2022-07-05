#!/bin/bash
dir=$(pwd)
if ! [ -e /opt/PROG24 ]; then
sudo mkdir /opt/PROG24
fi
cd $dir
sudo cp prog24 /opt/PROG24
sudo chmod u+x /opt/PROG24/prog24
sudo cp Prog24cXX.desktop /opt/PROG24
sudo cp 24Cxx_icon64.png /opt/PROG24
sudo cp 24Cxx_icon48.png /opt/PROG24
sudo cp 24Cxx_icon32.png /opt/PROG24
sudo cp 99-CH341.rules /opt/PROG24
cd  /opt/PROG24
sudo cp Prog24cXX.desktop /usr/share/applications
if [ -e $HOME/"Рабочий стол" ]; then 
cp Prog24cXX.desktop $HOME/"Рабочий стол"
fi
if [ -e $HOME/Desktop ]; then 
cp Prog24cXX.desktop $HOME/Desktop
fi
sudo cp 99-CH341.rules /etc/udev/rules.d
sudo chmod 777 *
