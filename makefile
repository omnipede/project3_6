OBJECTS= cm.tab.o lex.yy.o util.o symtab.o analyze.o main.o
CC = gcc
CFLAGS = -Wall -c
TARGET = project3_6

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

main.o: cm.tab.h main.c
	$(CC) $(CFLAGS) main.c

util.o: cm.tab.h util.c
	$(CC) $(CFLAGS) util.c

symtab.o: cm.tab.h symtab.c
	$(CC) $(CFLAGS) symtab.c

analyze.o: cm.tab.h analyze.c
	$(CC) $(CFLAGS) analyze.c

cm.tab.o: cm.tab.c
	$(CC) $(CFLAGS) cm.tab.c

cm.tab.c: cm.y
	bison -dv cm.y

cm.tab.h: cm.y
	bison -dv cm.y

lex.yy.o: cm.tab.h lex.yy.c
	$(CC) $(CFLAGS) lex.yy.c

lex.yy.c: tiny.l
	flex tiny.l

clean:
	rm -rf *.o $(TARGET) lex.yy.c cm.tab.c cm.tab.h cm.output
