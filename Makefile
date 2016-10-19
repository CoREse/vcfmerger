CC=g++
CPPFLAGS= -Wall -O3
LDFLAGS=
LIBS=

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
