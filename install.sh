#!/bin/bash

echo "Building glass"

chmod +x build.sh
./build.sh

# sudo cp extra/glass.desktop /usr/share/xsessions/glass.desktop

echo "Setting up .glass"

mkdir -p ~/.glass

touch ~/.glass/log
touch ~/.glass/rc.sh
chmod +x ~/.glass/rc.sh

cp glass ~/.glass/glass
chmod +x ~/.glass/glass

cp extra/xinitrc ~/.glass/xinitrc
chmod +x ~/.glass/xinitrc
cp extra/glass.conf ~/.glass/glass.conf
