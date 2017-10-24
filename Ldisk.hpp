//
//  Ldisk.cpp
//  Project1
//
//  Created by K&R on 10/2/17.
//  Copyright © 2017 K&Rchk. All rights reserved.
//

#include <stdio.h>
#include <iostream>

class Ldisk{
    
    
public:
    Ldisk();
    
    char ldisk[64][64];
    void initial_ldisk();
    void initial_bm(char *ldisk);
    void read_block(int i, char* p);
    void write_block(int i, char* p);
    void print_info();
    
    void write_to_file(char *file_name);
    int restore_ldisk(char *file_name);
    
};

Ldisk::Ldisk(){
    memset(ldisk, 0, sizeof(ldisk));
}



void Ldisk::read_block(int i, char *p){
    memcpy(p, this->ldisk[i], sizeof(ldisk[0]));   // Haven't tested
}

void Ldisk::write_block(int i, char *p){
    size_t x = sizeof(ldisk[0]);
    memcpy(this->ldisk[i],p,sizeof(ldisk[0]));   //Haven't tested
}

void Ldisk::initial_bm(char *ldisk){
    std::cout << "This is Ldisk class" << std::endl;
}

void Ldisk::print_info(){
    std::cout << "This is Ldisk class" << std::endl;
}

void Ldisk::write_to_file(char *file_name){
    std::ofstream ofile;
    ofile.open(file_name, std::ios::out | std::ios::binary);
    ofile.write(ldisk[0], sizeof(ldisk));
    ofile.close();
}

int Ldisk::restore_ldisk(char *file_name){
    //if return value is 1, the ldisk is restored
    //if return value is -1, the ldisk does not exist and need to initial a new one
    int status = 1;
    std::ifstream ifile;
    ifile.open(file_name, std::ios::in | std::ios::binary);
    if(ifile.is_open() == false){
        return -1;
    }
    ifile.read(ldisk[0], sizeof(ldisk));
    ifile.close();
    return status;
}
