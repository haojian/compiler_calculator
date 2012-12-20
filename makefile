calculator: calculator.o scan.o parse.o analyze.o codegen.o hashtable.o
	g++ -g -o calculator calculator.o scan.o parse.o analyze.o codegen1.o hashtable.o

CPstring.o: CPstring.c CPstring.h
	g++ -g -c -w CPstring.c

hashtable.o: hashtable.cc globals.h
		g++ -g -c -w hashtable.cc

scan.o: scan.cc globals.h
	g++ -g -c -w scan.cc

parse.o: parse.cc globals.h
	g++ -g -c -w parse.cc

analyze.o: analyze.cc globals.h
	g++ -g -c -w analyze.cc

codegen.o: codegen1.cc globals.h
	g++ -g -c -w codegen1.cc

calculator.o: calculator.cc globals.h
	g++ -g -c -w calculator.cc

clean:
	rm -f *.o calculator core
