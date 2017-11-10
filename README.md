# miniMatlab
A small compiler analogous to Matlab.

This project was done as a part of Compilers Lab. The compiler is made for a new 
language named miniMatlab which makes use of the International Standard ISO/IEC 9899:1999 (E) to 
support basic arithmetic operations. 
  1)  The Intermediate Code Generator writes the semantic actions in Bison to translate a miniMatlab
program into an array of 3-address quad’s, a supporting symbol table, and other auxiliary
data structures. The translation is machine independent and includes lexical and phase structure grammar
specification.

      Use the makefile for generating the a.out file or quads.out file.
