/**
 * bin.cpp
 *
 * (c) Copyright 2010, P. Jakubco
 *
 * This module handles the loading of textual binary representation of RAM
 * program.
 *
 */
 
#include <iostream>
#include <fstream>
#include "bin.hpp"

using namespace std;

RAMBin::RAMBin() {
    this->ram_size = -1;
    this->program = NULL;
}

RAMBin::~RAMBin() {
    if (this->program != NULL)
        delete [] this->program;
    this->ram_size = -1;
}

/**
 * This function loads a file with RAM program in memory.
 *
 * @param filename - file name
 * @return address of the loaded buffer
 */
bool RAMBin::bin_load(const char *filename) {
    ifstream fin(filename, ios::binary);
    int size;

    if (!fin.is_open()) {
        cout << "Error: can't open file name '" << filename
                << "'." << endl;
        return false;
    }

    /* get the file size (resulting binary should be less than this) */
    fin.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = fin.tellg();
    fin.seekg(0, std::ios_base::end);
    size = static_cast<int>(fin.tellg() - begin_pos);
    fin.seekg(0,ios_base::beg);

//    cout << "RAM program size: " << size +1 << endl;
    if (this->program != NULL)
        delete [] this->program;
    
    this->program = new unsigned char[size + 1];

    if (this->program == NULL) {
  //      fin.close();
        cout << "Error: can't allocate memory for the program." << endl;
        return false;
    }

    ram_size = this->bin_parse(&fin, size + 1);
    //fin.close(); // CAUSING BUG..

    if (ram_size <= 0) {
        delete [] this->program;
        cout << "Error: can't parse the file." << endl;
        return false;
    }
    return true;
}

/**
 * This function parses the file and convert it to binary file into the
 * buffer. Each RAM instruction is 1 byte long.
 *
 * @param fin - opened input file
 * @param max_size - maximal loadable size
 * @return -1 if the parsing failed, positive value (binary size) if it is OK.
 */
int RAMBin::bin_parse(ifstream *fin, const int max_size) {
    if (this->program == NULL)
        return 0;
    if (!fin->is_open())
        return 0;
    //fin.unsetf(ios_base::skipws);

    int p = 0;
    int t;
    while (!fin->eof()) {
        if (p == max_size - 1) {
            cout << "Warning: Ignoring some input." << endl;
            break;
        }
        *fin >> t;
        this->program[p++] = t&0xFF; //(unsigned char) (t & 0xFF);
    }
    this->program[p-1] = 0;
    return p;
}

/**
 * This function shows binary representation of RAM program in mnemo-code.
 * The correct syntax is assumed.
 */
void RAMBin::bin_print() {
    if (this->program == NULL)
        return;
    int p = 0;
    int c;

    if (ram_size < 0) {
        cout << "Error: RAM program was not loaded properly." << endl;
        return;
    }

    do {
        c = this->program[p++];
        cout << p-1 << ": ";
        switch (c) {
            case 0: cout << "\tHALT" << endl;
                break;
            case 1: cout << "\tREAD " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 2: cout << "\tREAD *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 3: cout << "\tWRITE =" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 4: cout << "\tWRITE " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 5: cout << "\tWRITE *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 6: cout << "\tLOAD =" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 7: cout << "\tLOAD " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 8: cout << "\tLOAD *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 9: cout << "\tSTORE " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 10: cout << "\tSTORE *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 11: cout << "\tADD =" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 12: cout << "\tADD " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 13: cout << "\tADD *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 14: cout << "\tSUB =" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 15: cout << "\tSUB " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 16: cout << "\tSUB *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 17: cout << "\tMUL =" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 18: cout << "\tMUL " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 19: cout << "\tMUL *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 20: cout << "\tDIV =" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 21: cout << "\tDIV " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 22: cout << "\tDIV *" 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 23: cout << "\tJMP " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 24: cout << "\tJGTZ " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            case 25: cout << "\tJZ " 
                    << (unsigned int) this->program[p++] << endl;
                break;
            default:
                cout << "\t>> UNKNOWN INSTRUCTION <<" << endl;
                break;
        }
    } while (p < this->ram_size);
}

const char *RAMBin::load_file(const char *filename) {
    ifstream fin(filename, ios::binary);
    int size;

    if (!fin.is_open()) {
        cout << "Error: can't open file name '" << filename
                << "'." << endl;
        return false;
    }
  //  fin.unsetf(ios_base::skipws);
    
    /* get the file size (resulting binary should be less than this) */
    fin.seekg(0, std::ios_base::beg);
    std::ifstream::pos_type begin_pos = fin.tellg();
    fin.seekg(0, std::ios_base::end);
    size = static_cast<int>(fin.tellg() - begin_pos);
    fin.seekg(0,ios_base::beg);

    char *file = new char[size+1];

    if (file == NULL) {
        fin.close();
        cout << "Error: can't allocate memory for the file." << endl;
        return false;
    }

    int p = 0;
    char t;
    while (!fin.eof()) {
        t = fin.get();
        file[p++] = t&0xff; //(char)(t & 0xFF);
    }
    file[p-1] = 0;

    fin.close();
    return file;
}
