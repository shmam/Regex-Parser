
# making the regular executable
regular: regular.o pattern.o parse.o
	gcc regular.o pattern.o parse.o -o regular -lm

# making the regular object component
regular.o: regular.c pattern.h parse.h
	gcc -Wall -std=c99 -g -c regular.c

# making the pattern object component
pattern.o: pattern.c pattern.h
	gcc -Wall -std=c99 -g -c pattern.c

# making the parse object component
parse.o: parse.c parse.h
	gcc -Wall -std=c99 -g -c parse.c

clean:
	rm -f parse.o regular.o pattern.o
	rm -f regular
	rm -f output.txt
