objects = skynet.o mysql.local.o analyzer.o cal.module.o
objects_run_query = mysql.local.o run.query.o
cc = gcc -g
cflags = -Wall -I.. `mysql_config --cflags` -Wno-variadic-macros -Wno-missing-braces -Wno-gnu -Wno-pointer-sign -c -pedantic
lflags = -Wall
liblink = -L../miranda -L/usr/lib -lpthread -lmiranda_ground `mysql_config --libs`
exec = skynet.bin
exec_blacklist = skynet.blacklist.txt
exec_run_query = run.query.bin

all: $(objects) $(objects_run_query)
	rm -f $(exec_blacklist)
	$(cc) $(lflags) $(objects) -o $(exec) $(liblink)
	$(cc) $(lflags) $(objects_run_query) -o $(exec_run_query) $(liblink)

skynet.o: skynet.c analyzer.h cal.module.h
	$(cc) $(cflags) skynet.c

mysql.local.o: mysql.local.c mysql.local.h
	$(cc) $(cflags) mysql.local.c

analyzer.o: analyzer.c analyzer.h
	$(cc) $(cflags) analyzer.c

cal.module.o: cal.module.c cal.module.h mysql.local.h
	$(cc) $(cflags) cal.module.c

run.query.o: run.query.c mysql.local.h
	$(cc) $(cflags) run.query.c

clean:
	rm -f *.o
	rm -f $(exec)
