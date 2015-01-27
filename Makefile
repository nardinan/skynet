objects = skynet.o analyzer.o cal_module.o
cc = gcc -g
cflags = -Wall -I.. -Wno-variadic-macros -Wno-missing-braces -Wno-gnu -Wno-pointer-sign -c -pedantic
lflags = -Wall
liblink = -L../miranda -L/usr/lib64 -L/usr/lib -lpthread -lmiranda_ground
exec = skynet.bin

all: $(objects)
	$(cc) $(lflags) $(objects) -o $(exec) $(liblink)

analyzer.o: analyzer.c analyzer.h
	$(cc) $(cflags) analyzer.c

cal_module.o: cal_module.c cal_module.h
	$(cc) $(cflags) cal_module.c

skynet.o: skynet.c analyzer.h
	$(cc) $(cflags) skynet.c

clean:
	rm -f *.o
	rm -f $(exec)
