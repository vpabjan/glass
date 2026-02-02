#!/bin/sh

USER=$(id -un)
HOST=$(uname -n)



PS4="$USER@$HOST~# "

echo "[*] Installing Glass..."

[ -x ./glass ] || {
    echo "glass binary not found (run ./build.sh first)"
    exit 1
}


# binaries
echo "  -> Installing binaries..."
set -x
install -m755 glass "/bin/glass"
install -m755 glassbg "/bin/glassbg"
{ set +x; } 2>/dev/null

echo "  -> Installing scripts..."
set -x
install -m755 extra/start-glass "/bin/start-glass"
install -m755 extra/update-glass.sh "/bin/update-glass"
{ set +x; } 2>/dev/null

echo "  -> Installing session..."
set -x
install -d /usr/share/xsessions
install -m644 extra/glass.desktop "/usr/share/xsessions/glass.desktop"
{ set +x; } 2>/dev/null

# glass runtime meta folder

echo "  -> Installing extras..."
set -x
install -d -m755 /var/lib/glass
install -d -m755 /var/lib/glass/default
install -m644 extra/glass.conf "/var/lib/glass/default/glass.conf"
install -m644 extra/xinitrc "/var/lib/glass/default/xinitrc"
#install -m644 extra/wallpaper.jpg "/var/lib/glass/default/wallpaper.jpg"
{ set +x; } 2>/dev/null

echo "[✓] Install complete"
