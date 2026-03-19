#!/bin/sh

# Check for root
if [ "$(id -u)" -ne 0 ]; then
    echo "[-] Error: This script must be run as root. Please run with sudo."
    exit 1
fi

USER=$(id -un)
HOST=$(uname -n)

PS4="$USER@$HOST~# "

echo "[*] Installing Glass..."

[ -x ./build/glass ] || {
    echo "  -> glass binary not found!"
    exit 1
}

# binaries
echo "  -> Installing binaries..."
set -x

install -m755 build/glass "/bin/glass"
install -m755 build/glassbg "/bin/glassbg"

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


echo "  -> Installing globals..."
set -x

install -d -m755 /var/lib/glass
install -d -m755 /var/lib/glass/default

{ set +x; } 2>/dev/null


echo "  ---> Installing extras..."
set -x

install -m644 extra/glass.conf "/var/lib/glass/default/glass.conf"
install -m644 extra/xinitrc "/var/lib/glass/default/xinitrc"
install -m644 extra/wallpaper.jpg "/var/lib/glass/default/wallpaper.jpg"

{ set +x; } 2>/dev/null


echo "  ---> Installing license..."
set -x

install -m644 LICENSE "/var/lib/glass/LICENSE"

{ set +x; } 2>/dev/null


echo "  ---> Installing text files..."
set -x

install -m644 README.md "/var/lib/glass/README.md"
install -m644 RELEASE.md "/var/lib/glass/RELEASE.md"

{ set +x; } 2>/dev/null

echo "  ---> Setting version..."
set -x

install -m644 version "/var/lib/glass/version"

{ set +x; } 2>/dev/null

echo "[✓] Install complete"
