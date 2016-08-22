CROSS_COMPILE=arm-linux-

AS 	= $(CROSS_COMPILE)as
LD 	= $(CROSS_COMPILE)ld
CC 	= $(CROSS_COMPILE)gcc
NM 	= $(CROSS_COMPILE)nm
#CPP	= $(CROSS_COMPILE)cpp

OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy
STRIP	= $(CROSS_COMPILE)strip

#export 向下传导
export AS LD CC NM
export OBJDUMP OBJCOPY STRIP

CFLAGS := -Wall -O2 -g
CFLAGS += -I $(shell pwd)/include

LDFLAGS := -lm -lpthread
export CFLAGS LDFLAGS

TOP_DIR := $(shell pwd)
export TOP_DIR

TARGET := big-http

obj-y += big-http.o
obj-y += interface/
obj-y += format/
obj-y += system/

all :
	@ make -C ./ -f $(TOP_DIR)/Makefile.build
	@ echo "CC $(TARGET)"
	@ $(CC) $(LDFLAGS) -o $(TARGET) built-in.o

clean :
	rm -rf $(shell find -name "*.o")

distclean :
	rm -rf $(shell find -name "*.o.d")
	rm -rf $(shell find -name "*.o")
