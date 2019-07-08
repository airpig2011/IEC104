
TOPDIR = ../
INSTALLDIR = /usr/sbin
#LIBDIR = /lib/
MODULE_PATH=$(TOPDIR)/IEC10X
APP_PATH=$(TOPDIR)/test

CC = gcc

CFLAGS +=-I$(MODULE_PATH) -lpthread
CFLAGS +=-I$(APP_PATH)

OBJ=iec104_monitor


all:
	$(CC) $(CFLAGS) \
	$(MODULE_PATH)/Iec104.c $(MODULE_PATH)/PRIO_QUEUE_Iec10x.c $(MODULE_PATH)/Iec10x.c Iec104_Linux.c \
	main.c \
	-o $(OBJ) -lpthread

install:
	cp -rf ./$(OBJ) $(INSTALLDIR)

clean:
	rm -rf $(OBJ)
	

