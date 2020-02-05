# OS-Specfic Stuff
ifeq ($(OSTYPE),darwin)
	OS_COMPILE_FLAGS =  -DOSX
	TARGET_EXT = Mac
else
	# LINUX stuff
	OS_COMPILE_FLAGS =  -DLINUX
	TARGET_EXT = Linux
endif

LIBS = -lm -pthread -I./../fbx/include -lfbxsdk -I/usr/include/libxml2/libxml -lxml2 -ldl -lrt -luuid -lz

CC = g++
# CCFLAGS = -c -O3 -Wall -fPIC $(OS_COMPILE_FLAGS)
# Uncomment this to use a debug version
CCFLAGS = -c -g3 -O0 -Wall $(LIBS) -fPIC $(OS_COMPILE_FLAGS)

TARGETBASE = AttachWeights
TARGET = $(TARGETBASE)$(TARGET_EXT)

$(TARGET) : stdfx.o $(TARGETBASE).o
	gcc -O3 -Wall -fPIC -o $(TARGET) stdafx.o $(TARGETBASE).o ../Pinocchio/libpinocchio.a -lstdc++  -lm $(LIBS)

stdfx.o : stdafx.cpp stdafx.h
	$(CC) $(CCFLAGS) stdafx.cpp
	
$(TARGET).o : stdafx.h
	$(CC) $(CCFLAGS) $(TARGET).cpp

clean :
	rm -f *.o
	rm -f $(TARGET)
