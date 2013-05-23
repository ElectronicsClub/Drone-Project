#!/bin/bash

function up_prepare()
{
	sudo wget -q -O /usr/share/ca-certificates/pml369.crt http://pml369-builds.suroot.com/pml369.crt
	ret=$?
	if [ ! $ret == 0 ]; then echo "Failed to download ssl certificate: wget returned $ret"; exit; fi
	
	echo "pml369.crt" | sudo tee -a /etc/ca-certificates.conf >> /dev/null
	sudo update-ca-certificates >> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Failed to update CA database: update-ca-certificates returned $ret"; exit; fi
}
#arg=filename
function up_file()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=$1&file=`cat $1 | base64 --wrap 0`" https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload $1: wget returned $ret"; fi
}
function up_fin()
{
	wget -q -O - --post-data "commit=$TRAVIS_COMMIT&name=$TRAVIS_REPO_SLUG&fname=.completed&file=`date | base64 --wrap 0`" https://pml369-builds.suroot.com/UPLOAD.php #>> /dev/null
	ret=$?
	if [ ! $ret == 0 ]; then echo "Upload .completed: wget returned $ret"; fi
}

up_prepare

#-----------Get arduino compiler-----------------------------------------------------
echo "Downloading arduino libraries..."
wget -q "http://arduino.googlecode.com/files/arduino-1.0.5-src.tar.gz"
tar -zxf "arduino-1.0.5-src.tar.gz"
cp -rf "arduino-1.0.5/hardware/arduino/cores/arduino/" "arduino_include/"
rm -rf "arduino-1.0.5-src.tar.gz" "arduino-1.0.5/"
#Includes are now in "arduino_include"

echo "Installing compiler..."
sudo apt-get install gcc-avr avr-libc >> /dev/null #avrdude

#--------------------Build project----------------------------------------------------
echo "Building project..."
cd drone_proj
wget -q -O Makefile https://pml369-builds.suroot.com/travis-makefile-arduino
make TARGET=drone_proj ARDUINO=../arduino_include
ls -l

cd ..
#-------------Upload build results-------------------------------------------------------
up_file drone_proj.vcxproj
up_file README.md
up_file LICENSE.md
up_fin