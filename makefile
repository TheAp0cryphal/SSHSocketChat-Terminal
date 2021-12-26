all: lets-talk.c lets-talk.h
	gcc lets-talk.c -std=gnu11 -ggdb3 -pthread -o lets-talk

clean:
	rm -rf lets-talk

valgrind: all 
	valgrind --leak-check=full --show-leak-kinds=all -v ./lets-talk 6651 127.0.0.1 6652

