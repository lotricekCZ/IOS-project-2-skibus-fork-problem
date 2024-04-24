all:
	gcc -g -std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -pthread -o proj2 main.c