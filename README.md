# Concurrency-Bank

This repository contains my solution to a mini-project for my Operating Systems class. The goal of the project was to help us explore concurrency and multi-threading with C.
More detailed instructions can be found in the 'Mini-project-2.pdf' file.

## Instruction Summary

To compile the code, use

                                gcc -o bank bank.c -Wall -Werror -lpthread -lpt -O
                                
It ensures that the program compiles correctly. 

After it is compiled, it should be run with the `husband.txt` and `wife.txt` files as commandline arguments like so: 

                               ./bank husband.txt wife.txt
  
Running it without the arguments will result in an error.

A more extensive test suite can be found [here](https://github.com/ianakotey/ostep-projects/tree/master/concurrency-bank), along with a simple stub that I used as a basis for my solution.
