#!/bin/sh
wget https://buildroot.org/downloads/buildroot-2018.02.tar.bz2
tar -xjf buildroot-2018.02.tar.bz2
cp -f .config buildroot-2018.02/.config
cp -r package/* buildroot-2018.02/package
cd buildroot-2018.02
make