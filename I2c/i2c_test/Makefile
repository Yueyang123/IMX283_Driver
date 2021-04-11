
CFLAGS += 
obj := i2c
src := i2c.c
CC  := arm-fsl-linux-gnueabi-gcc

$(obj): $(src)
	$(CC) $(CFLAGS)  -o $@  $^

.PHONY: clean
clean:
	-rm $(obj)
