target=as6809

objects=addr.o \
	as.o \
	defop.o \
	err.o \
	eval.o \
	gen.o \
	globals.o \
	macro.o \
	optype.o \
	parseopt.o \
	recog.o \
	search.o \
	symbol.o

CC=gcc
DEFINES=
DBGFLAGS=-g
CFLAGS=$(DBGFLAGS) $(DEFINES) -Wall
LDFLAGS=$(DBGFLAGS)

$(target) : $(objects)
	$(CC) $(LDFLAGS) -o $(target) $(objects)


.PHONY : clean
clean :
	rm $(target) $(objects)


