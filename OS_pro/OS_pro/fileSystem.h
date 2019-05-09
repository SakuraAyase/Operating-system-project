
#include"dataStructure.h"
#include<iostream>
#include<iomanip>
#include<string>
using namespace std;


FILE * virtualDisk;

super * supInode;

inode * root;


int addBlock()
{
	int block_num = supInode->nextFreeBlock;
	supInode->freeBlock[block_num] = 0;
	supInode->freeBlockNum--;
	int i = block_num;

	for (; i < BLOCKSNUM; i++)
	{
		if (supInode->freeBlock[i] == 1)
		{
			supInode->nextFreeBlock = i;
			break;
		}
	}

	if (i == BLOCKSNUM)
	{
		for (int j = 0; j < BLOCKSNUM; j++)
		{
			if (supInode->freeBlock[j] == 1)
			{
				supInode->nextFreeBlock = j;
				break;
			}
		}
	}

	return block_num;
}

int addInode()
{
	int Inode_num = supInode->nextFreeInode;
	supInode->freeInode[Inode_num] = 0;
	supInode->freeInodeNum--;
	int i = Inode_num;

	for (; i < INODESNUM; i++)
	{
		if (supInode->freeInode[i] == 1)
		{
			supInode->nextFreeInode = i;
			break;
		}
	}

	if (i == INODESNUM)
	{
		for (int j = 0; j < INODESNUM; j++)
		{
			if (supInode->freeInode[j] == 1)
			{
				supInode->nextFreeInode = j;
				break;
			}
		}
	}
	return Inode_num;
}
	
void synchronization()
{
	fseek(virtualDisk, 0, SEEK_SET);

	fwrite(supInode, sizeof(super), 1, virtualDisk);
}

void initial_root()
{
	root = new inode();
	string name = "root";
	for (int i = 0; i < name.length(); i++)
		root->finode.name[i] = name[i];
	root->finode.name[name.length()] = '\0';
	cout << root->finode.name << endl;
	root->direct_addr[0] = addBlock();
	root->inode_num = addInode();
	cout << root->direct_addr[0] << " " << root->inode_num << endl;

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + root->inode_num*INODESIZE, SEEK_SET);
	
	fwrite(root, sizeof(inode), 1, virtualDisk);

	dir d = dir();
	d.dirNum = 1;
	for (int i = 0; i < name.length(); i++)
		d.direct[0].directName[i] = name[i];
	d.direct[0].directName[name.length()] = '\0';

	d.direct[0].inodeID = 0;

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE*INODESNUM+ root->direct_addr[0]*BLOCKSIZE, SEEK_SET);

	fwrite(&d, sizeof(dir), 1, virtualDisk);
	//cout << SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + root->direct_addr[0] * BLOCKSIZE << endl;
	synchronization();
}

void initial(char * path)
{
	supInode = new super();
	supInode->freeBlockNum = BLOCKSNUM;

	supInode->nextFreeBlock = 0;
	for (int i = 0; i < BLOCKSNUM; i++)
		supInode->freeBlock[i] = 1;

	supInode->freeInodeNum = INODESNUM;
	supInode->nextFreeInode = 0;
	for (int i = 0; i < INODESNUM; i++)
		supInode->freeInode[i] = 1;

	supInode->size = 16 * 1024 * 1024;


	virtualDisk = fopen(path, "r+");
	if (virtualDisk == NULL)
	{
		cout << "load error" << endl;
	}

	
	
	initial_root();
}

void load(char *path)
{
	virtualDisk = fopen(path, "r+");
	if (virtualDisk == NULL)
	{
		cout << "load error" << endl;
	}

	supInode = (super*)calloc(1, sizeof(super));

	fseek(virtualDisk, 0, SEEK_SET);
	fread(supInode, sizeof(super), 1, virtualDisk);

	root = (inode*)calloc(1, sizeof(inode));

	fseek(virtualDisk, SUPER_BLOCKS * 1024, SEEK_SET);
	fread(root, sizeof(inode), 1, virtualDisk);

	cout << "[" << root->finode.name<<"/ ]#";
	int i;
	cin >> i;
}