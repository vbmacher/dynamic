/**
 * bin.hpp
 *
 * (c) Copyright 2010, P. Jakubco
 *
 */

#ifndef __BIN_HANDLER__
#define __BIN_HANDLER__

#include <iostream>
#include <fstream>

class RAMBin {
private:
    int ram_size;           /* size of RAM binary program */
    unsigned char *program; /* the program itself as buffer */

    int bin_parse(std::ifstream *fin, const int max_size);
public:
    RAMBin();
    ~RAMBin();
    bool bin_load(const char *filename);
    void bin_print();
    int get_size() { return ram_size; }
    const unsigned char*get_program() { return program; }

    static const char *load_file(const char *filename);
};


#endif
