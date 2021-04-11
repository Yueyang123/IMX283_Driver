EXEC	= adc_test
OBJS    = adc_test.o

CROSS	= arm-fsl-linux-gnueabi-
CC	     = $(CROSS)gcc 
STRIP	= $(CROSS)strip
CFLAGS	= -Wall -g -O2 -lpthread

all:  clean $(EXEC)

$(EXEC):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)
	$(STRIP) $@

clean:
	-rm -f $(EXEC) *.o