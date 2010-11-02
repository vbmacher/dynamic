dynamic
-------

This project is a dynamic translator and emulator of RAM (Random Access Machine) programs. It was created
to 'test' my abilities to do a dynamic translator. I was not plannig to continue the project, but my interest
has been increased so now you may expect some inovations, additions and new techniques testing.

If you have any questions, just ask me.

License
-------

This project is released under GNU GPL v2 license.

Compile and Run
---------------

This project was developed under the Mingw implementation of GNU utilities for Windows. The DevC++ IDE was used.
To compile it under Linux, it should be possible by slight modifying the file `Makefile.win`.

To run it, type: `dynamic [ram_file]`. The `ram_file` is compiled RAM file. The structure is easy,
each instruction and operand is separated by single white character (`' '`, `'\\n'`, `'\\t'`, etc.). Each
instruction has assigned a number representation. That number is printed in ASCII decadic format, readable by user.
Operands must be numbers in that format, too.

The instructions and what numbers they have assigned, you find in the file `ram/ram.txt`. The other files in this
directory are NASM translations of each RAM instruction. I use the translation of these files for creating dynamic
code in the dynamic translation technique.

