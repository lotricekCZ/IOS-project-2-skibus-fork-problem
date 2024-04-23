all:
	gcc -g -std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -lpthread -o proj2 main.c