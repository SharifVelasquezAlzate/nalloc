#
# Students' Makefile for the Malloc Lab
#
TEAM = $(LOGNAME)
VERSION = 1
TRACEDIR = /home/CSTCIS/glancast/374class/malloclab/traces/
LOCALTRACEDIRNM = traces
LOCALTRACEDIR = traces/
CC = gcc
CFLAGS = -Wall -O2

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

EXEOPTS=-V -a
mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

tests:
	mdriver $(EXEOPTS) -t $(TRACEDIR)
test0: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace0.rep
test1: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace1.rep
test2: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace2.rep
test3: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace3.rep
test4: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace4.rep
test5: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace5.rep
test6: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace6.rep
test7: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace7.rep
test8: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace8.rep
test9: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace9.rep
test10: traces
	mdriver $(EXEOPTS) -f $(LOCALTRACEDIR)trace10.rep

traces:
	ln -s $(TRACEDIR) $(LOCALTRACEDIRNM)

handin:
	~glancast/msubmit $(TEAM)-$(VERSION) mm.c

clean:
	rm -f *~ *.o mdriver 

cleaner:
	rm -f *~ *.o mdriver traces
