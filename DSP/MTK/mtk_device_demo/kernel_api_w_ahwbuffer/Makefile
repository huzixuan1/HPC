# android-ndk-r21e can be downloaded: https://dl.google.com/android/repository/android-ndk-r21e-linux-x86_64.zip
CC = $(YOUR_NDK_PATH)/android-ndk-r21e/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang++

LIBS = -L../lib64 -L../lib64/AHWBuffer -L$(MTKAPU_ROOT_PATH)/obj

# Use -fsigned-char to make aarch64 compiler treat char as signed char.
CFLAGS = -std=c++11 -static-libstdc++ -fsigned-char -fuse-ld=gold

SOURCES := src/main.cpp ../lib64/AHWBuffer/AndroidMemoryHelper.cpp
INCLUDES := -I$(MTKAPU_ROOT_PATH)/include/dlopen/ -I$(MTKAPU_ROOT_PATH)/include/ -I../lib64/
.PHONY: build

build:
	$(CC) $(CFLAGS) $(SOURCES) $(INCLUDES) $(LIBS) -lnativewindow -o test_CVE_host_api


