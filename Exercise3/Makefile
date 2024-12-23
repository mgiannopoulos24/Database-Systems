all:	sort chunk

sort:
	@echo " Compile sort_main ...";
	mkdir -p ./build
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main -O2

chunk:
	@echo " Compile chunk_main ...";
	mkdir -p ./build
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/chunk_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/chuck_main -O2

clean:
	@echo " Clean build files ...";
	rm -f ./build/* *.db
