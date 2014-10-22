Rshell
======

##Author & Contributor List

Gillian Liu

##Program Overview

This program is a simple shell for Linux. It is capable of preforming most
of the commands that bash is able to do, but does not support built in
bash commands such as `cd`. It however has its own built in command exit to
terminate the program.

##Files
```
.:

Makefile

LICENSE

README.md

./src

./test
```
```
./src:

main.cpp
```
```
./tests

exec.script
```

##How to run

1. Clone this directory to your local machine.
2. `cd` into the Rshell directory.
3. Call `make`
4. `cd` into bin
5. Call Rshell

Rshell will open in the terminal and you will be able to use it.

##Program Operation

* `exit` causes the program to terminate

* Supported connecters are `||`, `&&`, `;`, and also the `#` for comments

##Bugs and Limitations

* Rshell will ignore any connecters in the beginning. It will also
ignore any that are in sequence except for the first one.

ex) `;|&;||&` will result in the program only seeing the `;`

* Rshell will also ignore any connecters at the end that are not `|`, `&`,
 or `#`
