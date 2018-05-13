# Makefile

# compiler to use
CC = cc

# flags to pass compiler
CFLAGS = -Wall -Wextra -pedantic -std=c99

# name for executable
EXE = kilo

# space-separated list of source files
SRCS = kilo.c

# default target
$(EXE): $(SRCS)
	$(CC) $(CFLAGS) -o $(EXE) $(SRCS)

# prevent make from doing something with a file named clean
.PHONY: clean

# housekeeping
clean:
	rm -f $(EXE) *.o
