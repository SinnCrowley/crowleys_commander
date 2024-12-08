#!/bin/bash

qmake
make
sudo make install

sudo chmod -R 755 /opt/crowleys_commander
sudo chown -R `id -u`:`id -g` /opt/crowleys_commander

sudo ln -sf /opt/crowleys_commander/bin/crowleys_commander /usr/bin/crowleys_commander
sudo cp crowleys_commander.desktop /usr/share/applications
