#!/bin/sh

sudo cp src/yaujd /etc/init.d/
sudo chmod 755 /etc/init.d/yaujd
sudo update-rc.d yaujd defaults
echo "Installed service 'yaujd' (run as root)!"
