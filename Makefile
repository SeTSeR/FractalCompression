CC=g++
CFLAGS=-c -Wall

all: main
	
main: main.o extension.o colorspaces.o bmplib.o fcompression.o measure.o
	$(CC) main.o extension.o colorspaces.o bmplib.o fcompression.o measure.o -oapp
	rm -rf *.o

tests: tester.o extension.o colorspaces.o bmplib.o fcompression.o measure.o
	$(CC) tester.o extension.o colorspaces.o bmplib.o fcompression.o measure.o -otester
	rm -rf *.o

main.o: src/main.cpp
	$(CC) -c $(CFLAGS) src/main.cpp

extension.o: src/extension.cpp
	$(CC) -c $(CFLAGS) src/extension.cpp

colorspaces.o: src/colorspaces.cpp
	$(CC) -c $(CFLAGS) src/colorspaces.cpp

bmplib.o: src/bmplib.cpp
	$(CC) -c $(CFLAGS) src/bmplib.cpp

fcompression.o: src/fcompression.cpp
	$(CC) -c $(CFLAGS) src/fcompression.cpp

measure.o: src/measure.cpp
	$(CC) -c $(CFLAGS) src/measure.cpp

tester.o: src/tester.cpp
	$(CC) -c $(CFLAGS) src/tester.cpp

clean:
	rm -rf *.o app tester