CC=g++
CPPFLAGS= -g -Wall -O3 -fopenmp
LDFLAGS=
LIBS=

PREFIX=/usr/bin

VCFMERGER_OBJS=vcfmerger.o main.o
VCFMERGER_HEADERS=vcfmerger.h
LIBCRE=crelib/libcre.a
LIBCRE_OBJS=crelib/*

all: vcfmerger

vcfmerger:$(VCFMERGER_OBJS) $(LIBCRE)
	$(LINK.cpp) $^ -o $@

$(VCFMERGER_OBJS): $(VCFMERGER_HEADERS)

$(LIBCRE):$(LIBCRE_OBJS)
	cd crelib && $(MAKE)

clean:
	rm *.o vcfmerger

install:all
	install -d $(PREFIX)
	install vcfmerger $(PREFIX)

remove:
	rm $(PREFIX)/vcfmerger
