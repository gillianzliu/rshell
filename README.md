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
ignore any that are in sequence except for the first one. Unless the
first one is a `|` or `&` in which case if the syntax is incorrect will give
you an error.

ex) `;|&;||&` will result in the program only seeing the `;` and `||;&`
will result in the or conecter being used but `||&;&` will result in an error.

* Rshell will also ignore any connecters at the end that are not `|`, `&`, or
`#`. In the case of both `|` and `&` they will both cause errors at the end
of the statement regardless of whether there are two and will not excecute
the statement immediately preceeding it.

* Rshell will give an error if there are not exactly two `|` or `&`
connecters, and will give an error if there is any other connecters
immediately following. However this is only for those two connecters,
and will not give an error if the first one is a `;`

ex) `&&|` will give an error but `;&&&` or `;|` will not.
