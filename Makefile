build:
	[ -d ./bin ] || mkdir ./bin
	gcc -o ./bin/star main.c logs.c tar.c commands.c