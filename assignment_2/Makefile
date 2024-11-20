###################################################
#
# file: Makefile
#
# @Author:   Anastasis Lolas
# @Version:  3-11-2024
# @email:    csd5197@csd.uoc.gr
#
# Makefile for Assignment 2
#
####################################################

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -pthread

# Executable name
TARGET = assignment2

# Target to build the executable
$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile .c files to .o object files
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

# Clean up build files
clean:
	-rm -f $(TARGET) *.o
