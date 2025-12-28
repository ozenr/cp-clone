CC = gcc
main: main.c
	$(CC) main.c -ggdb3 -o run -Wall -Wextra -pedantic -std=c17