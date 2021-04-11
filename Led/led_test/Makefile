EXEC	= led_test
OBJS    = led_test.o

CROSS	= arm-none-linux-gnueabi-
CC	     = $(CROSS)gcc 
STRIP	= $(CROSS)strip
CFLAGS	= -Wall -g -O2

all:  clean $(EXEC)

$(EXEC):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	$(STRIP) $@

clean:
	-rm -f $(EXEC) *.o
