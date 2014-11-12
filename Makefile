G++ = g++ -Wall -Werror -ansi -pedantic -g
all: rshell ls cp
rshell: src/main.cpp
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	$(G++) src/main.cpp -o bin/rshell
cp: src/cp.cpp src/Timer.h
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	$(G++) src/cp.cpp -o bin/cp
ls: src/ls.cpp
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	$(G++) src/ls.cpp -o bin/ls
