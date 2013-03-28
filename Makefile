all: compile

compile: main.c *.h
	clang -Wall -Wpedantic -c main.c -o /dev/null

clean:
	rm -f *.o *~

.PHONY: clean
