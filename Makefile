# Makefile for the securit_cam project

#This refers to the cross-compiler for the bifferboard
CC	= ~/biffdev/openwrt/staging_dir/toolchain-i386_gcc-4.1.2_uClibc-0.9.30.1/bin/i486-openwrt-linux-gcc

#build target for the bifferboard
motion-detectd:
	$(CC) -mtune=i386 motion-detectd.c motion-detect.c motion-detect.h -o motion-detect

#build target for the development machine
motion-detectd-dev:
	gcc -g -o motion-detectd motion-detectd.c motion-detect.c motion-detect.h
