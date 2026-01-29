#!/bin/bash

cd ~/.glass

# cleanup
rm -rf glass

# download, build glass
git clone https://github.com/vpabjan/glass.git
cd glass
chmod +x build.sh
./build.sh
cd ..
cp glass/glass glass
