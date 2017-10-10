//
//  OFT.cpp
//  Project1
//
//  Created by K&R on 10/5/17.
//  Copyright © 2017 K&Rchk. All rights reserved.
//

#include <stdio.h>
#include <iostream>

class OpenFileTable{
    
    
    
public:
    char rw_buffer[64];
    int index;
    int position;
    int length;
    
    
    OpenFileTable();
    void set_index(int i);
    void set_position(int p);
    void set_length(int l);
};


OpenFileTable::OpenFileTable(){
    memset(rw_buffer, 0, sizeof(rw_buffer));
    index = -1;
    position = 0;
    length = 0;
}

void OpenFileTable::set_index(int i){
    index = i;
}

void OpenFileTable::set_length(int l){
    length = l;
}

void OpenFileTable::set_position(int p){
    position = p;
}


