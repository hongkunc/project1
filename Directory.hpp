//
//  Directory.hpp
//  Project1
//
//  Created by K&R on 10/2/17.
//  Copyright © 2017 K&Rchk. All rights reserved.
//

#ifndef Directory_hpp
#define Directory_hpp

#include <stdio.h>
#include <iostream>

#endif /* Directory_hpp */


class Directory{
    
    
public:
    int directory[16];   //  ONLY USE 2*8!!!!!!!!!!
    int a=3;
    
    
    Directory();
    void print_info();
    
    //Set symbolic name and descriptor index
    void set_symbolic_name(int position,char *name, size_t size);
    void set_descriptor_index(int position, int index);
    
    //Print file name and descriptor index
    void print_file_name(int position);
    void print_file_descriptor_index(int position);
    void print_all_file(std::ofstream &output_file);
    
    //Helper function
    int find_file(char* name);    //if file exist return the index, else return -1
    bool compare_name(char *name1, char* name2);
    void return_file_name(char *name, char* rename,size_t size);
    void write_to_file();
    
    void get_dir_from_ofts(char *p);
    
    //find free entry
    int find_free_entry();
};


Directory::Directory(){
    
    memset(directory, 0, sizeof(directory));
    std::cout << "Dir size: " << sizeof(directory) << std::endl;
}


void Directory::print_info(){
    std::cout << "This is Diectory class" << std::endl;
}

void Directory::set_symbolic_name(int position, char *name, size_t size){
    memcpy(&this->directory[2*position+0],name,size);
}

void Directory::set_descriptor_index(int position, int index){
    this->directory[2*position+1] = index;
}

void Directory::print_file_descriptor_index(int position){
    std::cout << "Descriptor index is: " << this->directory[2*position+1] << std::endl;
}

void Directory::print_file_name(int position){
     std::cout << "File name: " <<reinterpret_cast<char*>(&this->directory[2*position+0]) << std::endl;
}

void Directory::print_all_file(std::ofstream &output_file){
    char temp[5];
    for(int i=0; i<8; i++){
        return_file_name(reinterpret_cast<char*>(&this->directory[2*i+0]), temp, sizeof(temp));
        if(temp[0] != '\0')
            std::cout << temp << " ";
            output_file << temp << " ";
    }
}



bool Directory::compare_name(char *name1, char *name2){
    if(strcmp(name1,name2) == 0)
        return true;
    return false;
}

int Directory::find_file(char *name){
    int result = -1;
    int i=0;
    for(;i<8;i++){
        char temp[5];
//        memcpy(temp,reinterpret_cast<char*>(&this->directory[2*i+0]),sizeof(temp));
//        temp[4] = '\0';
        return_file_name(reinterpret_cast<char*>(&this->directory[2*i+0]), temp, sizeof(temp));
        
        if(compare_name(name, temp))
            result = i;
    }
    return result;
}

void Directory::return_file_name(char *name, char *rename,size_t size){
    memcpy(rename,name,size);
    rename[4] = '\0';
}

void Directory::write_to_file(){
    std::ofstream ofile;
    ofile.open("directory.txt");
    for(int i=0; i<8; i++){
        char temp[5];
        return_file_name(reinterpret_cast<char*>(&this->directory[2*i+0]), temp, sizeof(temp));
        if(temp[0] != '\0'){
            ofile << temp << std::endl;
            ofile << this->directory[2*i+1] << std::endl;
        }
    }
    ofile.close();
}

void Directory::get_dir_from_ofts(char *p){
    memcpy(&this->directory[0],p,sizeof(directory));
}


int Directory::find_free_entry(){
    int index = -1;
    
    for(int i=0; i<8; i++){
        if(directory[2*i+1] == 0)
            return i;
    }
    
    return index;
}
