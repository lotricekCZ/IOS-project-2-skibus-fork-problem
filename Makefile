all:
	gcc -O3 -std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -pthread -o proj2 main.c