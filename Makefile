CC            = gcc
CFLAGS        = -Wall
SOURCE		= wii.c getch.c Bluetooth_init.c DS3test.c
DEPEND		= header.h 
INCLUDES 	  = 
LIBS          = 
PROGRAM       = DS3test


all:			$(PROGRAM)

$(PROGRAM):	$(OBJS)
				$(CC) $(SOURCE) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(INCLUDES) -lbluetooth -lm -pthread

.c.o:
				$(CC) $(CFLAGS) $(INCLUDES) -c $<


clean:
				rm -f *.o *~ $(PROGRAM)