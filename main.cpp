//
//  main.cpp
//  Project1
//
//  Created by K&R on 9/29/17.
//  Copyright © 2017 K&Rchk. All rights reserved.
//

#include <fstream>
#include <iostream>
#include <cstring>
#include "Ldisk.hpp"
#include "Directory.hpp"
#include "OFT.hpp"

#define NUMBER_OF_BLOCK 64   //ldisk has 64 blocks
#define BLOCK_SIZE  64       // block size is 64 bytes = 16 integers
#define BM_SIZE 64           // Bit map's size is 64
#define DIRECTORY_ENTRY_SIZE 2    // Each directory entry has size of 2 integers, total 24 entries
#define DESCRIPTOR_SIZE 4    // Descriptor size is 4 integers, total is 24 descriptor
#define NUMBER_OF_DESCRIPTOR 24 
#define START_INDEX_OF_DATA_BLOCK 7  //Block 0 is for BM, next 6 blocks are for descriptors.



// Helper function

void set_symbolic_name(int position, char *name, int *dir,size_t size){
    memcpy(&dir[DIRECTORY_ENTRY_SIZE*position],name,size);
}

int descriptor_index_to_descriptor_block(int descriptor_index){
    int descriptor_block_index = -1;
    
    descriptor_block_index = 1+descriptor_index/4;
    
    return descriptor_block_index;
}

int find_zero_bitmap(Ldisk *ldisk,long long int* mask){
    int block_index = -1;
    
    char buffer[64];
    ldisk->read_block(0, buffer);
    long long int bitmap[1];
    memcpy(&bitmap[0], buffer, sizeof(bitmap));
    
    for(int i=0; i<64; i++){
        long long int test = -1;
        test = bitmap[0]&mask[i];
        if(test == 0)
            return i;
    }
    
    return block_index;
}

void set_block_bitmap(Ldisk *ldisk, long long int* mask,int block_index){
    
    char buffer[64];
    ldisk->read_block(0, buffer);
    long long int bitmap[1];
    memcpy(&bitmap[0], buffer, sizeof(bitmap));
    
    bitmap[0] = bitmap[0]|mask[block_index];
    memcpy(buffer, bitmap, sizeof(bitmap));
    ldisk->write_block(0, buffer);
    
}

void free_block_bitmap(Ldisk *ldisk, long long int *mask2, int block_index){
    char buffer[64];
    ldisk->read_block(0, buffer);
    long long int bitmap[1];
    memcpy(&bitmap[0], buffer, sizeof(bitmap));
    
    bitmap[0] = bitmap[0]&mask2[block_index];
    memcpy(buffer, bitmap, sizeof(bitmap));
    ldisk->write_block(0, buffer);
}


int find_free_descriptor(Ldisk* ldisk){
    int index = -1;
    int counter = 0;
    for(int i=0; i<6; i++){
        int buffer[16];
        ldisk->read_block(1+i, (char *)buffer);
        for(int j=0; j<4; j++){
            if(buffer[4*j+1] != 0){
                counter++;
            }
            else{
                index = counter;
                return index;
            }
        }
    }
    
    
    return index;
}


void set_descriptor(Ldisk* ldisk,int index, int block_index){
    int block_number = index/4;
    int a_index = index%4;
    int buffer[16];
    ldisk->read_block(1+block_number, (char *)buffer);
    buffer[4*a_index] = 0;   //initial length is 0
    buffer[4*a_index+1] = block_index;
    ldisk->write_block(1+block_number, (char *)buffer);
}

void change_descriptor_length(Ldisk* ldisk, int index, int new_length){
    int block_number = index/4;
    int a_index = index%4;
    int buffer[16];
    ldisk -> read_block(1+block_number, (char *)buffer);
    buffer[4*a_index] = new_length;
    ldisk->write_block(1+block_number, (char *)buffer);
}

void initial_ldisk(Ldisk* ldisk,long long int* mask){
    //initial bitmap
    int buffer[16];
    long long int bitmap[1];
    ldisk->read_block(0, (char *)buffer);
    memcpy(bitmap, buffer, sizeof(bitmap));
    for(int i=0; i<7;i++){
        bitmap[0] = bitmap[0]|mask[i];
    }
    memcpy(buffer,bitmap,sizeof(bitmap));
//    std::cout << "Here!!!\n" << std::endl;
    ldisk->write_block(0, (char *)buffer);
    
    
    //initial directory's descriptor
    set_descriptor(ldisk, 0, 7);
    //set bitmap for directory
    set_block_bitmap(ldisk, mask, 7);
    
}

void initial_ofts(Ldisk* ldisk, OpenFileTable* oft){
    //Initial open file table
    //The first slot is reserved for directory
    oft[0].set_index(0);
    oft[0].set_position(0);
    oft[0].set_length(0);
}

int allocate_new_data_block(Ldisk *ldisk, long long int* mask, int descriptor_index){
    int status = -1;
    
    int free_block_index = find_zero_bitmap(ldisk, mask);
    if(free_block_index != -1){
        set_block_bitmap(ldisk, mask, free_block_index);
        
        //update descriptor
        int temp[16];
        int d_block_index = 1+descriptor_index/4;
        int a_index = descriptor_index%4;
        ldisk->read_block(d_block_index, (char *)temp);
        for(int i=0; i<3; i++){
            if(temp[4*a_index+1+i] == -1 || temp[4*a_index+i+1] == 0){
                temp[4*a_index+1+i] = free_block_index;
                ldisk->write_block(d_block_index, (char*)temp);
                status = free_block_index;
                return status;
            }
        }
    }
    
    return status;  //if return -1, allocating new block fail
}

int position_to_buffer_position(int position){
    int buffer_position = 0;
    if(position<64)
        buffer_position = position;
    if(position >= 64 && position<128)
        buffer_position = position-64;
    if(position >= 128)
        buffer_position = position-128;
    
    return buffer_position;
}

int get_data_block_index(Ldisk *ldisk, int descriptor_index, int which_block){
    int result = -1;
    
    int temp[16];
    int d_block_index = 1+descriptor_index/4;
    int a_index = descriptor_index%4;
    ldisk->read_block(d_block_index, (char *)temp);
    result = temp[4*a_index+which_block];
    
    return result;
}

//Commands

int create_file(char *name, Ldisk *ldisk, Directory* dir, OpenFileTable* ofts){
    //If file already exists or the directory if full return -1, else return the index of file
    
    //Find free descriptor
    int index = -1;
    
    index = find_free_descriptor(ldisk);
    if(index == -1){
        std::cout << "No free descriptor" << std::endl;
        return index;
    }
    else{
        std::cout << "Descriptor index is: " << index << std::endl;
        set_descriptor(ldisk, index, -1); //-1 means this descriptor is occupied
    }
    
    // Find free dirctory entry
    int block_list[3];
    int temp[16];
    ldisk->read_block(1, (char *)temp);
    for(int i=0; i<3; i++){
        block_list[i] = temp[i+1];
    }
    
    for(int i=0; i<3; i++){
        if(block_list[i] != 0){
            ldisk->read_block(block_list[i], ofts[0].rw_buffer);
            dir->get_dir_from_ofts(ofts[0].rw_buffer);
            int dir_index = dir->find_free_entry();
            if(dir_index != -1){
                std::cout << "Find free entry: " << dir_index << std::endl;
                dir->set_symbolic_name(dir_index, name, sizeof(name));
                dir->set_descriptor_index(dir_index, index);
                ldisk->write_block(block_list[i], (char *)dir->directory);
                
                //change the length of directory
                ofts[0].length += 8;
                change_descriptor_length(ldisk, 0, ofts[0].length);
                return index;
            }
        }
    }
    
    
    
    
    return index;
}

int destory_file(char *name, Ldisk *ldisk, Directory *dir, OpenFileTable *ofts, long long int *mask2){
    int status = -1;
    int descriptor_index = -1;
    //search in directory
    int block_list[3];
    for(int i=0; i<3; i++)
        block_list[i] = get_data_block_index(ldisk, 0, i+1);
    
    for(int i=0; i<3; i++){
        if(block_list[i] != 0){
            ldisk->read_block(block_list[i], ofts->rw_buffer);
            dir->get_dir_from_ofts(ofts->rw_buffer);
            int dir_index = dir->find_file(name);
            if(dir_index != -1){
                std::cout << "Find file in directory" << std::endl;
                descriptor_index = dir->directory[2*dir_index+1];
                dir->directory[2*dir_index+1] = 0;
                memcpy(ofts->rw_buffer, dir->directory, sizeof(dir->directory));
                ldisk->write_block(block_list[i], ofts->rw_buffer);
                break;
            }
        }
    }
    
    //no such file and return -1 to indicate error
    if(descriptor_index == -1)
        return -1;
    
    for(int i=0; i<3; i++)
        block_list[i] = get_data_block_index(ldisk, descriptor_index, i+1);
    
    //free descriptor
    int buffer[16];
    ldisk->read_block(descriptor_index_to_descriptor_block(descriptor_index), (char *)buffer);
    buffer[4*(descriptor_index%4)+0] = 0;   //set length to 0
    buffer[4*(descriptor_index%4)+1] = 0;   //set all data block to zero to free the descriptor
    buffer[4*(descriptor_index%4)+2] = 0;
    buffer[4*(descriptor_index%4)+3] = 0;
    
    ldisk->write_block(descriptor_index_to_descriptor_block(descriptor_index), (char *)buffer);
    
    //free bitmap
    for(int i=0; i<3; i++){
        if(block_list[i] != 0 && block_list[i]!= -1)
            free_block_bitmap(ldisk, mask2, block_list[i]);
    }
    
    status = 1;
    
    return status;
}

int open_file(Ldisk *ldisk, Directory* dir, OpenFileTable* ofts,char* file_name){
    int status = -1;
    int descriptor_index = -1;
    //Find file in directory
    int block_list[3];
    int temp[16];
    ldisk->read_block(1, (char *)temp);
    for(int i=0; i<3; i++){
        block_list[i] = temp[i+1];
    }
    for(int i=0; i<3; i++){
        if(block_list[i] != 0){
            ldisk->read_block(block_list[i], ofts[0].rw_buffer);
            dir->get_dir_from_ofts(ofts[0].rw_buffer);
            int dir_index= dir->find_file(file_name);
            if(dir_index != -1){
                std::cout << "find file in directory" << std::endl;
                descriptor_index = dir->directory[2*dir_index+1];
                std::cout << "descriptor index is : " << descriptor_index << std::endl;
                break;
            }
        }
    }
    
    //find free oft slot
    int oft_index = -1;
    for(int i=0;i<4; i++){
        if(ofts[i].index == -1){
            std::cout << "Find free OFT slot: " << i << std::endl;
            oft_index = i;
            break;
        }
    }
    
    if(oft_index != -1){
        ofts[oft_index].set_index(descriptor_index);
        ofts[oft_index].set_position(0);
        
        int length;
        int block_list[3];
        int temp[16];
        ldisk->read_block(1+descriptor_index/4, (char *)temp);
        length = temp[4*(descriptor_index%4)];
        for(int a=0; a<3; a++){
            block_list[a] = temp[4*(descriptor_index%4)+a+1];
        }
        
        ofts[oft_index].set_length(length);
        if(block_list[0] != -1 and block_list[0] !=0){
            ldisk->read_block(block_list[0], ofts[oft_index].rw_buffer);
        }
        
        return oft_index;
    }
    
    
    
    return status;
}

int close_file(Ldisk *ldisk, OpenFileTable* ofts){
    int status = -1;
    
    int temp[16];
    ldisk->read_block(1+(ofts->index)/4, (char *)temp);
    
    int block_index = (ofts->position)/64;
    int in_block_index = 4*(ofts->index % 4)+block_index+1;
    
    if(temp[in_block_index] != 0 and temp[in_block_index] != -1)
        ldisk->write_block(temp[in_block_index], ofts->rw_buffer);
    
    change_descriptor_length(ldisk, ofts->index, ofts->length);
    
    ofts->set_index(-1);
    
    return status;
}

void print_directory(Ldisk *ldisk,Directory *dir){
    int block_list[3];
    int temp[16];
    
    ldisk->read_block(1, (char *)temp);
    for(int i=0; i<3; i++)
        block_list[i] = temp[i+1];
    
    for(int i=0; i<3; i++){
        if(block_list[i] != 0){
            ldisk->read_block(block_list[i], (char *)temp);
            dir->get_dir_from_ofts((char *)temp);
            dir->print_all_file();
        }
    }
}

int seek_position(Ldisk *ldisk, OpenFileTable *ofts, int new_position){
    int status = -1;
    
    if(new_position > ofts->length)
        return status;
    
    
    int block_list[3];
    int temp[16];
    ldisk->read_block(1+(ofts->index)/4, (char *)temp);
    for(int i=0; i<3; i++)
        block_list[i] = temp[4*((ofts->index)%4)+i+1];
    
    if(new_position/64 != (ofts->position)/64){
        ldisk->write_block(block_list[(ofts->position)/64], ofts->rw_buffer);
        ldisk->read_block(block_list[new_position/64], ofts->rw_buffer);
        ofts->set_position(new_position);
        status = 1;
    }
    
    return status;
}

int write_file(char content, int count, int descriptor_index,Ldisk *ldisk, OpenFileTable *ofts, long long int* mask){
    int status = -1;
    int progress = 0;
    int old_position = ofts->position;
    
    //check if exceed the max length
    if(count+old_position > 64*3){
        count = 64*3-old_position;
    }
    
    //If this file does not have any data block yet, allocate one and update file descriptor
    if(ofts->length == 0){
        allocate_new_data_block(ldisk, mask, descriptor_index);
    }
    
    //write to buffer
    if(progress < count){
        int buffer_position = position_to_buffer_position(ofts->position);
        for(int i=0; i<count; i++){
            
            //fetch next block
            if(buffer_position >= 64){
                int data_block_index = get_data_block_index(ldisk, ofts->index, (ofts->position-1)/64+1);
                ldisk->write_block(data_block_index, ofts->rw_buffer);
                if(get_data_block_index(ldisk, ofts->index, ofts->position/64+1) == 0){
                    int new_block_index = allocate_new_data_block(ldisk, mask, ofts->index);
                    ldisk->read_block(new_block_index, ofts->rw_buffer);
                    buffer_position = 0;
                }else{
                    data_block_index = get_data_block_index(ldisk, ofts->index, (ofts->position/64+1));
                    ldisk->read_block(data_block_index, ofts->rw_buffer);
                    buffer_position = 0;
                }
            }
            
            ofts->rw_buffer[buffer_position] = content;
            buffer_position++;
            progress++;
            
            if(ofts->position != 191)
                ofts->position++;
            
            
            if(progress == count){
                int data_block_index = get_data_block_index(ldisk, ofts->index, ofts->position/64+1);
                ldisk->write_block(data_block_index, ofts->rw_buffer);
                
                //update file length
                if(ofts->length == 0){
                    ofts->length = count;
                    change_descriptor_length(ldisk, ofts->index, count);
                }else{
                    int new_length = old_position+count>ofts->length ? old_position+count:ofts->length;
                    ofts->length = new_length;
                    change_descriptor_length(ldisk, ofts->index, new_length);
                }
                return progress;
            }
            
        }
        
        
    }
    
    return status;
}

int read_file(int descriptor_index, int count, Ldisk *ldisk, OpenFileTable *ofts, char *read_buffer){
    int status = -1;
    int progress = 0;
    int old_position = ofts->position;
    
    
    if(progress < count){
        int buffer_position = position_to_buffer_position(ofts->position);
        for(int i=0; i<count; i++){
            
            //fetch next block
            if(buffer_position >=64){
                int data_block_index = get_data_block_index(ldisk, ofts->index, (ofts->position-1)/64+1);
                ldisk->write_block(data_block_index, ofts->rw_buffer);
                if(get_data_block_index(ldisk, ofts->index, ofts->position/64+1) == 0)
                    return progress;
                else{
                    data_block_index = get_data_block_index(ldisk, ofts->index, ofts->position/64+1);
                    ldisk->read_block(data_block_index, ofts->rw_buffer);
                    buffer_position = 0;
                }
            }
            
            read_buffer[progress] = ofts->rw_buffer[buffer_position];
            buffer_position++;
            progress++;
            
            if(ofts->position != 191)
                ofts->position++;
            
            if(progress == count){
                return progress;
            }
        }
    }
    
    return status;
}


int main(int argc, const char * argv[]) {
    // insert code here...
    
    OpenFileTable ofts[4];  //ofts[0] is always for directory
    for(int i=0; i<4; i++){
        ofts[i].set_index(-1);
    }
    ofts[0].set_index(0);
    ofts[0].set_position(0);
    ofts[0].set_length(0);
    
    
    // initial ldisk

    
    // Block 0 is allocated for Bitmap


//---------------------------------------------------------------------------------------
//--------------------------------------- initial Bit Mask ------------------------------
//---------------------------------------------------------------------------------------
//    // mask[i] = the ith bit is 1,  mask2[i] = the ith bit is 0
    long long int mask[64];
    mask[63] = 1;
    for(int i=0; i<63; i++){
        mask[62-i] = mask[62-i+1]<<1;
    }
    
    long long int mask2[64];
    for(int i=0; i<64; i++){
        mask2[i] = ~mask[i];
    }
    
    
    
    
    

    char test[5] = "abc";
    char name1[5] = "efg";
    
    
    
    Ldisk test_ldisk;
    test_ldisk.print_info();
    
    //test_ldisk.write_block(7, name1);
    
    Directory test_dir;
    
    

    initial_ldisk(&test_ldisk, mask);
    int tbuffer[16];
    test_ldisk.read_block(1, (char *)tbuffer);
    
    create_file("qwe", &test_ldisk, &test_dir, ofts);

   
    
    create_file(test, &test_ldisk, &test_dir, ofts);
    
    create_file(name1, &test_ldisk, &test_dir, ofts);

    open_file(&test_ldisk, &test_dir, ofts, test);
    
    find_zero_bitmap(&test_ldisk, mask);
    
    print_directory(&test_ldisk,&test_dir);
    
    std::cout << 63/64 << std::endl;
    
    seek_position(&test_ldisk, &ofts[0], 16);
    
    open_file(&test_ldisk, &test_dir, ofts, "qwe");
    
    //close_file(&test_ldisk, &ofts[2]);
    
    //get_data_block_index(&test_ldisk, 0, 2);
    
    write_file('x', 50, ofts[1].index, &test_ldisk, &ofts[1], mask);
    write_file('y',40,ofts[1].index,&test_ldisk,&ofts[1], mask);
    
    
    char read_buffer[192];
    seek_position(&test_ldisk, &ofts[1], 45);
    write_file('a', 30, ofts[1].index, &test_ldisk, &ofts[1], mask);
    seek_position(&test_ldisk, &ofts[1], 40);
    read_file(ofts[1].index, 50 , &test_ldisk, &ofts[1], read_buffer);
    
    write_file('a', 5, ofts[2].index, &test_ldisk, &ofts[2], mask);
    write_file('k',100,ofts[1].index,&test_ldisk,&ofts[1], mask);
    
    
    
    destory_file("qwe",&test_ldisk,&test_dir,&ofts[0],mask2);
    std::cout << find_zero_bitmap(&test_ldisk, mask) << std::endl;
    
    return 0;
}
