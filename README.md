Rshell
======

##Program Overview

This program is a simple shell for Linux. It is capable of preforming most
of the commands that bash is able to do, but does not support built in
bash commands or interpretations. It does however have some own built in
commands such as `exit` to terminate the program.

##How to run

1. Clone this directory to your local machine.
2. `cd` into the Rshell directory.
3. Call `make`
4. `cd` into bin
5. Call Rshell
6. To run the `ls` use the command `bin/ls` and any flags or parameters
7. To run `cp` use the command `bin/cp arg1 arg2 (optional flag)` the flag
can be in any order with reference to the arguments, but the optional flag must
 be `-a` and the source file must be before the destination file.

Rshell will open in the terminal and you will be able to use it.

##Program Operation

###Rshell

* `exit` causes the program to terminate

* Supported connecters are `||`, `&&`, `;`, and also the `#` for comments

* Piping, input redirection, and output redirection are supported
  * If there are multiple redirections for input or output, as in `< file < file < file`,
  then the input redirection will only be taken from the last file.
  * If there is both piping and input or output redirection, redirection will take
  priority and override the piping.
  * If there is any file descriptor specific redirection ie. `2>`, then if the
  file directors overlap, the normal ones take priority.
  * `<<<` is supported along with `fd#>` and `fd#>>`

* The command `cd` will change the working directory to the home directory if none is specified. If there is one, it must be specified by its full path name from the current directory after the `cd` command

* `^C` will cause the foreground process to stop if there is one, but `rshell` will not be exited.

* `^Z` will cause the foreground process to be stopped and will output a message with an identifying number as well as what the process is. `^Z` will give the process the lowest unused identifying number starting from `1`.

* `bg` is a command that will list all processes in the background with their identifying number and process name.

* `fg` is a command that will continue the background process with the latest identifying number. It can also take in an additional parameter which is a positive number, corresponding to a processes identifying number. If this optional parameter is given, it will resume the process which corresponds to the identifying number given.

###ls

* `-a`, `-l`, and `-R` are supported, along with file and folder parameters

* Directories are colored in bold blue, hidden files with a gray background,
executable files in green, and symbolic links in light blue.

###cp

* It will move the contents of the source file to the destination file.

* The destination file must not already exist or there will be an error.

* `-a` is the only accepted optional flag, it cannot contain anything after
or before the `a`. If `-a` if given, `cp` will preform three types of copying
and output the time taken for each. Without, it will just preform the fastest of
the three.

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

* In `ls`, if a hidden file is in a column that is larger than itself, then
the entire width of the column will have a gray background.

* With the command `fd#>`, if the file descriptor number if greater than 2,
there will be a closing error.

* When using `<<<` with `cat` there is no newline character at the end and with
`wc` the numbers are off by 1.

* When using `grep` or `sed`, do not use quotes.

* for any of the built in commands, it will not give you an error if you input too many parameters, but will ignore them.
