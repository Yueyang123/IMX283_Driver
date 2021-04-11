
CFLAGS += 
obj := spi
src := spi.c
CC  := arm-fsl-linux-gnueabi-gcc

$(obj): $(src)
	$(CC) $(CFLAGS)  -o $@  $^

.PHONY: clean
clean:
	-rm $(obj)