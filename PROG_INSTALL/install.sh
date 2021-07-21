#!/bin/bash
dir=$(pwd)
cd ~
if ! [ -e $HOME/PROG24 ]; then
mkdir PROG24
fi
cd $dir
cp prog24 ~/PROG24
cp Prog24cXX.desktop ~/PROG24/
cp 24Cxx_icon64.png ~/PROG24/
cp 24Cxx_icon48.png ~/PROG24/
cp 24Cxx_icon32.png ~/PROG24/
cp 99-CH341.rules ~/PROG24/
cd  ~/PROG24/
if [ -e $HOME/"Рабочий стол" ]; then 
cp Prog24cXX.desktop $HOME/"Рабочий стол"/
fi
if [ -e $HOME/Desktop ]; then 
cp Prog24cXX.desktop $HOME/Desktop/
fi
sudo cp 99-CH341.rules /etc/udev/rules.d/
chmod 777 *

