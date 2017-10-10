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
    ofile.open(file_name);
    for(int i=0; i<64; i++){
        ofile.write(ldisk[i], sizeof(ldisk[0]));
    }
    ofile.close();
}
