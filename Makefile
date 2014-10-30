all:
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	g++ -Wall -Werror -ansi -pedantic src/main.cpp -o bin/rshell
rshell:
	if [ ! -d "./bin" ];\
	then mkdir bin;\
	fi
	g++ -Wall -Werror -ansi -pedantic src/main.cpp -o bin/rshell
ls:
	g++ -Wall -Werror -ansi -pedantic src/ls.cpp -o bin/ls
