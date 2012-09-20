OPTS=-I/usr/include/lua5.1/ -I/job/src/firebird-lua-udfs/src/md5-1.4/

all:
	gcc -O3 -fpic -c -Wall luaudf.c $(OPTS)
	#gcc -shared -W1,-soname,luaudf.so -o luaudf.so luaudf.o /usr/lib/liblua5.1.a -lib_util
	gcc -shared -W1,-soname,luaudf.so -o luaudf.so luaudf.o -lm -llua5.1 -lib_util

clean:
	rm -f *.o *.so

install: clean all
	cp luaudf.so /opt/firebird/UDF/
