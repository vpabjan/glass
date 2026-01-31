#!/bin/sh

echo "[*] Installing glass"

[ -x ./glass ] || {
    echo "glass binary not found (run ./build.sh first)"
    exit 1
}

# binaries

install -m755 glass "/usr/share/glass"
install -m755 glassbg "/usr/share/glassbg"

install -m644 extra/glass.conf "/etc/glass.conf"

mkdir -p $HOME/.glass
touch $HOME/log
touch $HOME/rc.sh
#install -m644 extra/glass.conf "$HOME/.glass/glass.conf"


# assets
#install -m644 extra/wallpaper.jpg "$SHARE/wallpaper.jpg"
install -m644 extra/xinitrc "$HOME/.glass/xinitrc"
#install -m644 version "/version"

# scripts
install -m755 extra/start-glass "/usr/share/start-glass"
install -m755 extra/update-glass.sh "/usr/share/update-glass"
#install -m755 extra/update.sh "$LIB/update.sh"

mkdir -p /usr/share/xsessions
install -m644 extra/glass.desktop "/usr/share/xsessions/glass.desktop"


echo "[✓] Install complete"
