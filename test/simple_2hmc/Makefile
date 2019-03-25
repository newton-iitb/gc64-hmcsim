#!/bin/sh
#
# HMCSIM TEST APPLICATIONS 
#
# TEST: SIMPLE
# ---------------------------------------

include ../Makefile.inc

SRC=./src

all: simple_2hmc
$(SRC)/simple_2hmc.o: $(SRC)/simple_2hmc.c
	$(CC) $(CFLAGS) -c -o $@ $(SRC)/simple_2hmc.c

$(SRC)/execute_test.o: $(SRC)/execute_test.c
	$(CC) $(CFLAGS) -c -o $@ $(SRC)/execute_test.c

simple_2hmc: $(SRC)/simple_2hmc.o $(SRC)/execute_test.o
	$(CC) $(CFLAGS) -o simple_2hmc $(SRC)/simple_2hmc.o $(SRC)/execute_test.o $(LDFLAGS) $(LDLIBS)

clean: 
	rm -Rf ./simple_2hmc ./src/*.o
