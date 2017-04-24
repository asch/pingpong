all: mtpong shmempong

shmempong: shmempong.c
	clang -std=gnu11 -o shmempong shmempong.c

mtpong: mtpong.c
	clang -lpthread -pthread -std=gnu11 -o mtpong mtpong.c

clean:
	rm -f mtpong shmempong
