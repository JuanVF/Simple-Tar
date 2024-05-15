# run this command to build the binary file
build:
	[ -d ./bin ] || mkdir ./bin
	gcc -o ./bin/star main.c logs.c tar.c commands.c

# run this command to test if the program is fully working
test: build
	rm *.tar || echo "no tar file"
	./bin/star -cvf output.tar test/archivito.txt test/log1 test/log2
	./bin/star -tvf output.tar
	./bin/star -xvf output.tar
	./bin/star -uvf output.tar test/modified/archivito.txt
	./bin/star -pvf output.tar
	./bin/star -xvf output.tar
	ls -lh | grep archivito.txt
	tail -n 2 archivito.txt
	./bin/star -rvf output.tar test/log3
	./bin/star --delete -vf output.tar archivito.txt
	./bin/star -xvf output.tar
	./bin/star -tvf output.tar
	# rm *.tar archivito.txt log1 log2 log3