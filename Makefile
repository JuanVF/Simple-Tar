# run this command to build the binary file
build:
	[ -d ./bin ] || mkdir ./bin
	gcc -o ./bin/star main.c logs.c tar.c commands.c

# run this command to test if the program is fully working
test: build
	rm *.tar || echo "no tar file"
	./bin/star -cvf output.tar test/archivito.txt test/hola_mundo.py
	./bin/star -tvf output.tar
	./bin/star -xvf output.tar
	python3 hola_mundo.py
	./bin/star -uvf output.tar test/modified/archivito.txt
	./bin/star -xvf output.tar
	ls -lh | grep archivito.txt
	tail -n 2 archivito.txt
	rm *.tar archivito.txt hola_mundo.py