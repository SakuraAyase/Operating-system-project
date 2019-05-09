#pragma once
#include<string>
#include<vector>
#include<iostream>


#define BLOCKSNUM 16128
#define BLOCKSIZE 1024

#define INODESNUM 1024
#define INODESIZE 128

#define INODE_BLOCKS 128
#define SUPER_BLOCKS 128

#define DIRECTNAME 14
#define DIRNUM 63

#define MAXNAME 20

using namespace std;

class file_info
{
public:
	long int fileSize;
	long int dirSize;
	char name[MAXNAME];
};

class inode
{
public:
	file_info finode;
	inode *parent;
	int inode_num;
	int direct_addr[10];
	int indirect_addr;

	inode()
	{
		inode_num = 0;
		indirect_addr = -1;
		for (int i = 0; i < 10; i++)
			direct_addr[i] = -1;
		parent = this;
	}
};


class super
{
public:
	unsigned int size;				//the size of the disk
	unsigned int freeBlock[BLOCKSNUM];	//the stack of the free block
	unsigned int nextFreeBlock;		//the pointer of the next free block in the stack
	unsigned int freeBlockNum;//the totally number of the free block in the disk

	unsigned int freeInode[INODESNUM];	//the stack of the free node
	unsigned int freeInodeNum;			//the totally number of the free inode in the disk
	unsigned int nextFreeInode;		//the next free inode in the stack

};

struct direct
{
	char directName[DIRECTNAME];
	unsigned short int	inodeID;
};

struct dir
{
	int	dirNum;
	direct direct[DIRNUM];
};

