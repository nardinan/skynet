objects = skynet.o mysql.local.o analyzer.o cal_module.o
cc = gcc -g
cflags = -Wall -I.. `mysql_config --cflags` -Wno-variadic-macros -Wno-missing-braces -Wno-gnu -Wno-pointer-sign -c -pedantic
lflags = -Wall
liblink = -L../miranda -L/usr/lib -lpthread -lmiranda_ground `mysql_config --libs`
exec = skynet.bin

all: $(objects)
	$(cc) $(lflags) $(objects) -o $(exec) $(liblink)

skynet.o: skynet.c analyzer.h cal_module.h
	$(cc) $(cflags) skynet.c

mysql.local.o: mysql.local.c mysql.local.h
	$(cc) $(cflags) mysql.local.c

analyzer.o: analyzer.c analyzer.h
	$(cc) $(cflags) analyzer.c

cal_module.o: cal_module.c cal_module.h mysql.local.h
	$(cc) $(cflags) cal_module.c

clean:
	rm -f *.o
	rm -f $(exec)
