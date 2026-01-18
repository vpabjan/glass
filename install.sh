#!/bin/bash

echo "Building glass"

chmod +x build.sh
./build.sh

echo "Setting up .glass"

mkdir -p ~/.glass
touch ~/.glass/log
touch ~/.glass/rc.sh
chmod +x ~/.glass/rc.sh
cp glass ~/.glass/glass
cp extra/glass.conf ~/.glass/glass.conf
cp extra/xinitrc ~/.glass/xinitrc
chmod +x ~/.glass/xinitrc
