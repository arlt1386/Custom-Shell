[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/NX-NsY2z)
# CS441/541 Shell Project

## Author(s):
Susanna Arlt, Leroy 

## Date:
10/17/25

## Description:
Creates a shell that has a few built in commands and uses fork to do others. Prints a summary of jobs when ended.

## How to build the software
interactive mode or batch mode, handle those commands, launch each job, fork them, print summary.

## How to use the software
Run ./mysh with no args to enter interactive mode that is like a classic shell. Type exit or crtl + D to end program. 
Run ./mysh with <file-name> to enter batch mode. This will execute all the commands in the file, print out the summary, then terminate

## How the software was tested
tests/file7.txt was used as input for file tests/fav-cookie.c to test file redirection. 
tests/more-than-one-command.txt and tests/more-than-one-command.txt check for ability to run more than one command. 
tests/file2.txt Mix of foreground, background, and built-in commands. 

## Known bugs and problem areas
file redirection doesn't work properly. 
when inputting "ls tests" into interactive mode with a file (ex: ./file2, containing "ls tests"), says not a directory, works if you type it in normally. 
builtin_history doesn't work in interactive mode. 
built_infg is buggy in interactive mode. 
running summary is incorrect. 


