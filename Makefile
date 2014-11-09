G++ = g++ -Wall -Werror -ansi -pedantic
all: rshell ls
rshell: src/main.cpp
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	$(G++) src/main.cpp -o bin/rshell
#cp: src/cp.cpp src/Timer.H
#	if [ ! -d "./bin" ];\
#	then mkdir bin;\
#	fi
#	$(G++) src/cp.cpp -o bin/cp
ls: src/ls.cpp
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	$(G++) src/ls.cpp -o bin/ls
