#!/bin/bash

echo "Building glass"

./build.sh

# echo "Adding Xsession"

# sudo cp extra/glass.desktop /usr/share/xsessions/glass.desktop

echo "Setting up .glass"

mkdir -p ~/.glass
touch ~/.glass/log
touch ~/.glass/rc.sh
cp glass ~/.glass/glass
cp extra/xinitrc ~/.glass/xinitrc
chmod +x ~/.glass/xinitrc
cp start-glass ~/.local/bin/start-glass
chmod +x ~/.local/bin/start-glass
