# Concurrency-Bank

This repository contains my solution to a mini-project for my Operating Systems class. The goal of the project was to help us explore concurrency and multi-threading with C.

## Instruction Summary

To compile the code, use

                                gcc -o bank bank.c -Wall -Werror -lpthread -lpt -O
                                
It ensures that the program compiles correctly. 

After it is compiled, it should be run with the `husband.txt` and `wife.txt` files as commandline arguments like so: 

                               ./bank husband.txt wife.txt
  
Running it without the arguments will result in an error.


A more extensive test suite can be found [here](https://github.com/ianakotey/ostep-projects/tree/master/concurrency-bank), along with a simple stub that I used as a basis for my solution. More detailed instructions about the project can be found there as well.

There's definitely room for improvement in my solution, and if you find anyway it could be improved, feel free to reach out to me for a chat through my [personal website](https://excel-chukwu.netlify.app/).
