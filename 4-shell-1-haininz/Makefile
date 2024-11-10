CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror

PROMPT = -DPROMPT

CC = gcc

33sh: sh.c
	#TODO: compile your program, including the -DPROMPT macro
	$(CC) $(CFLAGS) $(PROMPT) sh.c -o 33sh

33noprompt: sh.c
	#TODO: compile your program without the prompt macro
	$(CC) $(CFLAGS) sh.c -o 33noprompt

.PHONY: all clean

all: 33sh 33noprompt

clean:
	#TODO: clean up any executable files that this Makefile has produced
	rm -f 33sh 33noprompt
