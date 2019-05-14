
#include"dataStructure.h"
#include<iostream>
#include<iomanip>
#include<string>
#include<regex>
#include<random>
#include<fstream>
#include<stdio.h>
#include<stdlib.h>
#define NAME_LENGTH 100
#define BUFFER_SIZE 1<<20


using namespace std;

string currentString;

FILE * virtualDisk;

super * supInode;

inode * root;

inode * current;
string path1;

char buffer[BUFFER_SIZE];

void reInitialization()
{
	fclose(virtualDisk);
	virtualDisk = fopen("vm.dat", "rb+");
}

void synchronization()
{
	reInitialization();
	fseek(virtualDisk, 0, SEEK_SET);

	fwrite(supInode, sizeof(super), 1, virtualDisk);

	fseek(virtualDisk, BLOCKSIZE*SUPER_BLOCKS, SEEK_SET);
	fwrite(root, sizeof(inode), 1, virtualDisk);

}

string getTime(long int timeStamp)
{
	time_t timer;
	timer = timeStamp;
	struct tm *p;
	p = gmtime(&timer);
	char s[80];
	strftime(s, 80, "%Y-%m-%d %H:%M:%S", p);
	//printf("%s", s);
	string a = string(s);
	return a;
}

dir* readBlock_toDir(int block)
{
	reInitialization();
	dir* pointer = (dir*)calloc(1, sizeof(dir));
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + block * BLOCKSIZE, SEEK_SET);
	fread(pointer, sizeof(dir), 1, virtualDisk);
	return pointer;
}

inode* readInode(int block)
{
	reInitialization();
	inode* pointer = (inode*)calloc(1, sizeof(inode));
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * block, SEEK_SET);
	fread(pointer, sizeof(inode), 1, virtualDisk);
	return pointer;
}

indirect_address* readBlock_indirect(int block)
{
	reInitialization();
	indirect_address* pointer = (indirect_address*)calloc(1, sizeof(indirect_address));
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + BLOCKSIZE*block, SEEK_SET);
	fread(pointer, sizeof(indirect_address), 1, virtualDisk);
	return pointer;
}

file_struct* readBlock_file(int block)
{
	reInitialization();
	file_struct* pointer = (file_struct*)calloc(1, sizeof(file_struct));
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + BLOCKSIZE * block, SEEK_SET);
	fread(pointer, sizeof(file_struct), 1, virtualDisk);
	return pointer;
}

int addBlock()
{
	int block_num = supInode->nextFreeBlock;
	supInode->freeBlock[block_num] = 0;
	supInode->freeBlockNum--;

	for (int j = 0; j < BLOCKSNUM; j++)
	{
		if (supInode->freeBlock[j] == 1)
		{
			supInode->nextFreeBlock = j;
			break;
		}
	}
		


	synchronization();
	return block_num;
}

int addInode()
{
	int Inode_num = supInode->nextFreeInode;
	supInode->freeInode[Inode_num] = 0;
	supInode->freeInodeNum--;

	for (int j = 0; j < INODESNUM; j++)
	{
		if (supInode->freeInode[j] == 1)
		{
			supInode->nextFreeInode = j;
			break;
		}
	}


	synchronization();
	return Inode_num;
}
	


void initial_root()
{
	cout << "\tInitial :root: directory" << endl;
	root = new inode();
	string name = "root";
	for (int i = 0; i < name.length(); i++)
		root->finode.name[i] = name[i];
	root->finode.name[name.length()] = '\0';
	//cout << root->finode.name << endl;
	root->direct_addr[0] = addBlock();
	root->inode_num = addInode();
	//cout << root->direct_addr[0] << " " << root->inode_num << endl;

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + root->inode_num*INODESIZE, SEEK_SET);
	
	fwrite(root, sizeof(inode), 1, virtualDisk);

	dir* d = new dir();
	dir* d_test = new dir();

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE*INODESNUM+ root->direct_addr[0]*BLOCKSIZE, SEEK_SET);

	fwrite(d, sizeof(dir), 1, virtualDisk);
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + root->direct_addr[0] * BLOCKSIZE, SEEK_SET);

	//cout << SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + root->direct_addr[0] * BLOCKSIZE << endl;

	synchronization();
	cout << "\tInitial :root: directory finished" << endl;
}

void initial(char * path)
{
	cout << "Initialization" << endl;
	path1 = path;
	supInode = new super();
	supInode->freeBlockNum = BLOCKSNUM;
	cout << "Constructing Inodes " << INODESIZE*INODESNUM/BLOCKSIZE << "KB" << endl;
	supInode->nextFreeBlock = 0;
	for (int i = 0; i < BLOCKSNUM; i++)
		supInode->freeBlock[i] = 1;

	cout << "Constructing Blocks " << BLOCKSNUM << "KB" << endl;
	supInode->freeInodeNum = INODESNUM;
	supInode->nextFreeInode = 0;
	for (int i = 0; i < INODESNUM; i++)
		supInode->freeInode[i] = 1;

	supInode->size = 16 * 1024 * 1024;

	cout << "Constructing size: " << supInode->size / BLOCKSIZE << "KB" << endl;
	virtualDisk = fopen(path, "rb+");
	if (virtualDisk == NULL)
	{
		cout << "load error" << endl;
	}

	initial_root();
	cout << "Initialization finished" << endl << endl;
}

void load(char *path)
{
	cout << "Loading" << endl;
	path1 = path;
	virtualDisk = fopen(path, "r+");
	if (virtualDisk == NULL)
	{
		cout << "load error" << endl;
	}

	cout << "Loading superInode" << endl;
	supInode = (super*)calloc(1, sizeof(super));

	fseek(virtualDisk, 0, SEEK_SET);
	fread(supInode, sizeof(super), 1, virtualDisk);

	cout << "Loading Inode" << endl;
	root = readInode(0);
	currentString = "/" + string(root->finode.name);
	current = root;
	cout << "Current directory = " << currentString << endl << endl;

	cout << "Loading blocks" << endl;

	//cout << root->direct_addr[0] << endl;
	/*dir* d = (dir*)calloc(1, sizeof(dir));
	fseek(virtualDisk, SUPER_BLOCKS*BLOCKSIZE+INODESIZE*INODESNUM+root->direct_addr[0]*BLOCKSIZE, SEEK_SET);
	fread(d, sizeof(dir), 1, virtualDisk);*/
	//cout << d->dirNum << endl;
	//cout << "direct:"<<d->direct[0].directName << endl;
	//cout << "inode:" << d->direct[0].inodeID << endl;

	/*inode* id = readInode(d->direct[0].inodeID);*/

	//cout << "inode:" << id->finode.fileSize << endl;
	//cout << "dirsize:" << id->finode.dirSize << endl;
	/*char c = 'o';

	fseek(virtualDisk, SUPER_BLOCKS*BLOCKSIZE + INODESIZE * INODESNUM+BLOCKSIZE*id->direct_addr[0], SEEK_SET);
	fread(&c, sizeof(char), 1, virtualDisk);*/
	//cout << "c:" << c << endl;

	//cout << supInode->freeBlockNum << endl;
	//cout << supInode->freeInodeNum << endl;
	//cout << supInode->nextFreeBlock << endl;

	//inode* i = readInode(0);
	////cout << i->indirect_addr << endl;
	//i = readInode(1);
	////cout << i->indirect_addr << endl;
	//i = readInode(2);
	//cout << i->indirect_addr << endl;
	//i = readInode(3);
	//cout << i->indirect_addr << endl;

	cout << "Loading finished" << endl << endl;

}

int test_regex_match(string order)//≤‚ ‘ ‰»Î÷∏¡Ó
{
	vector<string> pattern{ "createFile ([a-z]|[A-Z]|[/_])* [0-9]*" }; // fixed telephone
	regex re(pattern[0]);

	vector<string> str{ "createFile /root/Afile 10"};


	for (auto tmp : str) {
		bool ret = std::regex_match(tmp, re);
		if (ret) fprintf(stderr, "%s, can match\n", tmp.c_str());
		else fprintf(stderr, "%s, can not match\n", tmp.c_str());
	}

	return 0;
}

int test_regex_match1(string order)//≤‚ ‘ ‰»Î÷∏¡Ó
{
	

	vector<string> pattern{
		"deleteFile ([a-z]|[A-Z]|[/_.]|[0-9])*",
		"createFile ([a-z]|[A-Z]|[/_.]|[0-9])* [0-9]*",
		"createDir ([a-z]|[A-Z]|[/_.]|[0-9])*",
		"deleteDir ([a-z]|[A-Z]|[/_.]|[0-9])*",
		"changeDir ([a-z]|[A-Z]|[/_.]|[0-9])*",
		"dir",
		"cp ([a-z]|[A-Z]|[/_.]|[0-9])* ([a-z]|[A-Z]|[/_.]|[0-9])*",
		"sum",
		"cat ([a-z]|[A-Z]|[/_.]|[0-9])*",
		"format",
		"help"
	};

	regex re;
	bool ret;
	for (int i = 0; i < pattern.size(); i++)
	{
		re = regex(pattern[i]);
		ret = regex_match(order, re);
		if (ret)
		{
			cout <<"order:"<< i + 1 << endl;
			return i + 1;
		}
	}
	cout << "order:" << 0 << endl;
	return 0;
}

vector<string>spliteString(string fileName,char pos)
{
	string s = fileName;
	char c = pos;
	string::size_type pos1, pos2;
	vector<string>v;
	if (fileName[0] == pos)
		s = fileName.substr(1, fileName.size());
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + 1;
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
	return v;
}

void cpBlock(int origin,int target)
{
	srand((unsigned)time(NULL));
	file_struct* file = readBlock_file(origin);
	file->index = target;
	
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
		target * BLOCKSIZE, SEEK_SET);
	//cout << block << endl;
	fwrite(file, sizeof(file_struct), 1, virtualDisk);



}

void addRandom2Block(int block)
{
	srand((unsigned)time(NULL));
	file_struct* file = new file_struct();
	file->index = block;
	for (int i = 0; i < FILESIZE; i++)
	{
		int flag = rand() % 3;
		switch (flag)
		{
		case 0:
			file->c[i] = 'A' + rand() % 26;
			break;
		case 1:
			file->c[i] = 'a' + rand() % 26;
			break;
		case 2:
			file->c[i] = '0' + rand() % 10;
			break;
		default:
			file->c[i] = 'x';
			break;
		}
		//cout << file->c[i];

	}
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
		block * BLOCKSIZE, SEEK_SET);
	//cout << block << endl;
	fwrite(file, sizeof(file_struct), 1, virtualDisk);
	
	

}

void formatting()
{
	string i;
	cout << "Confirm formatting all the disk? [yes/no]";
	cin >> i;
	if (i == "yes")
		return;
	
	
	char name[NAME_LENGTH];
	int sz = 0;
	printf("Please input the name of the target file.\n");
	scanf("%s", name);
	printf("Please input the size(MB) of the target file.\n");
	scanf("%d", &sz);
	FILE *fout = fopen(name, "wb");
	if (fout == NULL) {
		printf("Cannot create the file.");
		exit(-1);
	}
	printf("Wait...\n");
	for (int i = 0; i < sz; i++) {
		fwrite(buffer, BUFFER_SIZE, 1, fout);
	}
	fclose(fout);
	printf("Finish.\n");
}

void createFile(string fileName, int size)
{
	if (size > 10 + INDIRECT_ADDR)
	{
		cout << "the size of file out of range 0 - 255KB" << endl;
		return;
	}

	cout << "Current directory is:  "<<currentString<< endl;
	cout << "Create processing start:" << endl;
	vector<string>fileSets = spliteString(fileName, '/');
	inode *cur_inode;
	if (fileSets[0] == ".")
	{
		cur_inode = current;
	}
	else if(fileSets[0] == "root")
		cur_inode = root;
	else
		cur_inode = root;

	cur_inode->finode.dirSize += size;
	cout << "dirSize:" << cur_inode->finode.dirSize << endl;
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * cur_inode->inode_num, SEEK_SET);
	fwrite(cur_inode, sizeof(inode), 1, virtualDisk);

	string createFileName = fileSets[fileSets.size() - 1];
	
	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);
	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size() - 1; i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;
		
		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);


			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr!=-1)
			{ 
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}
			

		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}
		cur_inode = readInode(inode_addr);
		cur_inode->finode.dirSize += size;
		time_t timer;
		time(&timer);
		cur_inode->finode.modifyTime = timer;
		cout << "dirSize:" << cur_inode->finode.dirSize << endl;
		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * inode_addr, SEEK_SET);
		fwrite(cur_inode, sizeof(inode), 1, virtualDisk);
	}

	cout << endl;
	

	cout << "\tModify informations for directory." << endl;
	int addr;
	bool created1 = false;
	for (int j = 0; j < 10; j++)
	{

		addr = cur_inode->direct_addr[j];
		if (addr == -1)
		{
			dir* d = new dir();
			cur_inode->direct_addr[j] = addBlock();
			d->index = cur_inode->direct_addr[j];
			fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + 
				cur_inode->direct_addr[j] * BLOCKSIZE, SEEK_SET);
			fwrite(d, sizeof(dir), 1, virtualDisk);
			addr = cur_inode->direct_addr[j];
		}
		cur_direct = readBlock_toDir(addr);
		if (cur_direct->dirNum == DIRNUM)
		{
			continue;
		}
		
		else
		{
			cur_direct->direct[cur_direct->dirNum].inodeID = addInode();
			for (int si = 0; si < createFileName.size(); si++)
			{
				cur_direct->direct[cur_direct->dirNum].directName[si] = createFileName[si];
			}
			cur_direct->dirNum++;
			created1 = true;
			break;
		}
	}


	if (cur_inode->indirect_addr != -1 && created1 == false)
	{
		indirect_address* in = readBlock_indirect(cur_inode->indirect_addr);
		
		for (int j = 0; j < in->size; j++)
		{

			addr = in->addr[j];
			
			cur_direct = readBlock_toDir(addr);
			if (cur_direct->dirNum == DIRNUM)
			{
				continue;
			}

			else
			{
				cur_direct->direct[cur_direct->dirNum].inodeID = addInode();
				for (int si = 0; si < createFileName.size(); si++)
				{
					cur_direct->direct[cur_direct->dirNum].directName[si] = createFileName[si];
				}
				cur_direct->dirNum++;
				created1 = true;
				break;
			}
		}
	}
	

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
		addr * BLOCKSIZE, SEEK_SET);
	fwrite(cur_direct, sizeof(dir), 1, virtualDisk);
	
	cout << "\tCreating inode for new file." << endl;
	synchronization();
	inode* newInode = new inode();
	newInode->parent = cur_inode;
	newInode->finode.fileSize = size;
	newInode->inode_num = cur_direct->direct[cur_direct->dirNum - 1].inodeID;
	for (int si = 0; si < createFileName.size(); si++)
	{
		newInode->finode.name[si]= createFileName[si];
	}
	
	if (size <= 10)
	{
		cout << "\tCreating direct block" << endl;
		for (int i = 0; i < size; i++)
		{
			newInode->direct_addr[i] = addBlock();
			addRandom2Block(newInode->direct_addr[i]);
		}
	}
	else
	{
		cout << "\tCreating direct block" << endl;
		for (int i = 0; i < 10; i++)
		{
			newInode->direct_addr[i] = addBlock();
			addRandom2Block(newInode->direct_addr[i]);
		}
		newInode->indirect_addr = addBlock();
		
		indirect_address in;
		in.index = newInode->indirect_addr;
		in.size = 0;
		//cout << in->index << endl;
		
		cout << "\tCreating indirect block" << endl;
		for (int i = 0; i < size-10; i++)
		{
			int indir_addres = addBlock();
			in.addr[i] = indir_addres;
			//cout << in.addr[i] << endl;
			in.size++;
			addRandom2Block(indir_addres);
		}

		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
			newInode->indirect_addr * BLOCKSIZE, SEEK_SET);
		fwrite(&in, sizeof(indirect_address), 1, virtualDisk);
		

		/*indirect_address in1 = indirect_address();
		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
			newInode->indirect_addr * BLOCKSIZE, SEEK_SET);
		cout << fread(&in1, sizeof(indirect_address), 1,   virtualDisk) << endl;;
		if (ferror(virtualDisk)) { printf("File read error."); }
		
		for (int i = 0; i < in1.size; i++)
		{
			cout << in1.addr[i] << endl;
			
		}*/
		//cout << "in size:" << in->size << endl;
		//cout << newInode->inode_num << endl;
	}
	cout << "\tSaving changes" << endl;


	synchronization();
	fseek(virtualDisk, SUPER_BLOCKS * BLOCKSIZE + INODESIZE * newInode->inode_num, SEEK_SET);
	fwrite(newInode, sizeof(inode), 1, virtualDisk);

	//cout << newInode->inode_num << endl;

	//inode* i = (inode*)calloc(1,sizeof(inode));
	//fseek(virtualDisk, SUPER_BLOCKS * BLOCKSIZE + INODESIZE * newInode->inode_num, SEEK_SET);
	//fread(i, sizeof(inode), 1, virtualDisk);

	//cout << i->indirect_addr << endl;
	//cout << i->finode.createTime << endl;
	//cout << newInode->finode.createTime << endl;

	//cout << newInode->indirect_addr << endl;

	//

	//cout << supInode->freeBlockNum << endl;
	synchronization();
	cout << "Finish create processing: create " << fileName << endl;
}

void deleteFile(string fileName)
{
	vector<string>fileSets = spliteString(fileName, '/');
	inode *cur_inode;
	cout << "Current directory is:  " << currentString << endl;
	cout << "Delete processing start:" << endl;

	if (fileSets[0] == ".")
	{
		cur_inode = current;
	}
	else if (fileSets[0] == "root")
		cur_inode = root;
	else
		cur_inode = root;

	string deleteFileName = fileSets[fileSets.size() - 1];
	

	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);
	
	

	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size() - 1; i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;
		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);


			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}

		cur_inode = readInode(inode_addr);
	}
	cout << endl;

	cout << "\tFinding the file's location" << endl;
	int file_addr = -1;
	bool find = false;

	for (int j = 0; j < 10; j++)
	{
		int addr = cur_inode->direct_addr[j];
		if (addr == -1)
			continue;
		cur_direct = readBlock_toDir(addr);

		for (int n = 0; n < cur_direct->dirNum; n++)
		{
			if (cur_direct->direct[n].directName == deleteFileName)
			{
				file_addr = cur_direct->direct[n].inodeID;
				for (int inde = n; inde < cur_direct->dirNum; inde++)
				{
					cur_direct->direct[inde] = cur_direct->direct[inde + 1];
				}
				cur_direct->dirNum--;

				fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
					cur_direct->index * BLOCKSIZE, SEEK_SET);
				fwrite(cur_direct, sizeof(dir), 1, virtualDisk);

				find = true;
				break;
			}
		}

	}


	if (find == false)
	{
		if (cur_inode->indirect_addr != -1)
		{
			indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
			for (int j = 0; j < addrs->size; j++)
			{
				int addr = addrs->addr[j];
				cur_direct = readBlock_toDir(addr);

				for (int n = 0; n < cur_direct->dirNum; n++)
				{
					if (cur_direct->direct[n].directName == deleteFileName)
					{
						file_addr = cur_direct->direct[n].inodeID;
						for (int inde = n; inde < cur_direct->dirNum; inde++)
						{
							cur_direct->direct[inde] = cur_direct->direct[inde + 1];
						}
						cur_direct->dirNum--;

						fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
							addr * BLOCKSIZE, SEEK_SET);
						fwrite(cur_direct, sizeof(dir), 1, virtualDisk);

						find = true;
						break;
					}
				}
			}
		}


	}
	if (!find)
	{
		cout << "no such direction!" << endl;
		return;
	}

	cout << "\tModify informations for directory." << endl;
	/*cout << file_addr << endl;*/
	inode* deleteInode = readInode(file_addr);
	
	supInode->freeInodeNum++;
	
	supInode->freeInode[file_addr] = 1;
	inode* parentInode = deleteInode->parent;
	


	do
	{
		cout << "\tModify size of directory for " << deleteInode->parent->finode.name<< endl;
		time_t timer;
		time(&timer);
		parentInode->finode.modifyTime = timer;
		parentInode->finode.dirSize -= deleteInode->finode.fileSize;
		/*cout << "size:"<<parentInode->finode.dirSize << endl;
		cout << parentInode->inode_num << endl;*/
		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * parentInode->inode_num, SEEK_SET);
		fwrite(parentInode, sizeof(inode), 1, virtualDisk);
		//cout << parentInode->finode.name << endl;
		parentInode = parentInode->parent;

	} while (parentInode->inode_num != root->inode_num);
	cout << "\tDelete file " << deleteInode->finode.name << endl;
	/*cout << deleteInode->indirect_addr << endl;*/

	
	for (int i = 0; i < 10; i++)
	{

		if (deleteInode->direct_addr[i] == -1)
			continue;
		else
		{
			/*cout << "\tDelete file " << deleteFileName << endl;*/
			supInode->freeBlockNum++;
			supInode->freeBlock[deleteInode->direct_addr[i]] = 1;
			/*cout << supInode->freeBlockNum << endl;*/
		}
	}


	if (deleteInode->indirect_addr != -1)
	{
		cout << "\tDelete file indirect " << endl;
		indirect_address* inde = readBlock_indirect(deleteInode->indirect_addr);

		/*cout << deleteInode->indirect_addr << endl;*/
		supInode->freeBlockNum++;
		supInode->freeBlock[deleteInode->indirect_addr] = 1;
		/*cout << inde->size << endl;*/
		for (int i = 0; i < inde->size; i++)
		{
			supInode->freeBlockNum++;
			supInode->freeBlock[inde->addr[i]] = 1;
			/*cout << supInode->freeBlockNum << endl;*/
		}
	}
	/*cout << supInode->freeBlockNum << endl;*/

	synchronization();
	cout << "Finish delete processing: delete " << fileName << endl;
}

void createDirect(string fileName)
{
	vector<string>fileSets = spliteString(fileName, '/');
	inode *cur_inode;
	cout << "Current directory is:  " << currentString << endl;
	cout << "Create dir processing start:" << endl;

	if (fileSets[0] == ".")
	{
		cur_inode = current;
	}
	else if (fileSets[0] == "root")
		cur_inode = root;
	else
		cur_inode = root;

	string createDirName = fileSets[fileSets.size() - 1];

	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);

	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size() - 1; i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;
		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);

			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}

		cur_inode = readInode(inode_addr);
	}
	cout << endl;

	cout << "\tModify informations for directory." << endl;
	int addr;
	bool created1 = false;
	for (int j = 0; j < 10; j++)
	{

		addr = cur_inode->direct_addr[j];
		if (addr == -1)
		{
			dir* d = new dir();
		 	cur_inode->direct_addr[j] = addBlock();
			d->index = cur_inode->direct_addr[j];
			fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
				cur_inode->direct_addr[j] * BLOCKSIZE, SEEK_SET);
			fwrite(d, sizeof(dir), 1, virtualDisk);
			addr = cur_inode->direct_addr[j];
		}
		cur_direct = readBlock_toDir(addr);
		if (cur_direct->dirNum == DIRNUM)
		{
			continue;
		}

		else
		{
			cur_direct->direct[cur_direct->dirNum].inodeID = addInode();
			for (int si = 0; si < createDirName.size(); si++)
			{
				cur_direct->direct[cur_direct->dirNum].directName[si] = createDirName[si];
			}
			cur_direct->dirNum++;
			created1 = true;
			break;
		}
	}

	if (cur_inode->indirect_addr != -1 && created1 == false)
	{
		indirect_address* in = readBlock_indirect(cur_inode->indirect_addr);
		for (int j = 0; j < in->size; j++)
		{

			addr = in->addr[j];

			cur_direct = readBlock_toDir(addr);
			if (cur_direct->dirNum == DIRNUM)
			{
				continue;
			}

			else
			{
				cur_direct->direct[cur_direct->dirNum].inodeID = addInode();
				for (int si = 0; si < createDirName.size(); si++)
				{
					cur_direct->direct[cur_direct->dirNum].directName[si] = createDirName[si];
				}
				cur_direct->dirNum++;
				created1 = true;
				break;
			}
		}
	}

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
		addr * BLOCKSIZE, SEEK_SET);
	fwrite(cur_direct, sizeof(dir), 1, virtualDisk);

	cout << "\tCreating inode for new direct." << endl;
	inode* newInode = new inode();
	newInode->parent = cur_inode;

	newInode->inode_num = cur_direct->direct[cur_direct->dirNum - 1].inodeID;

	for (int si = 0; si < createDirName.size(); si++)
	{
		newInode->finode.name[si] = createDirName[si];
	}
	newInode->direct_addr[0] = addBlock();
	dir* d = new dir();
	d->index = newInode->direct_addr[0];

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM + d->index*BLOCKSIZE, SEEK_SET);
	fwrite(d, sizeof(dir), 1, virtualDisk);

	cout << "\tSaving changes" << endl;

	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * newInode->inode_num, SEEK_SET);
	fwrite(newInode, sizeof(inode), 1, virtualDisk);
	synchronization();

	cout << "Finish create processing: create " << fileName << endl;

}

void delete_File_usage(inode* deleteInode)
{
	cout << "\t\tStart delete file " << deleteInode->finode.name << endl;
	supInode->freeInode[deleteInode->inode_num] = 1;
	supInode->freeInodeNum++;

	for (int i = 0; i < 10; i++)
	{
		if (deleteInode->direct_addr[i] == -1)
			continue;
		else
		{
			supInode->freeBlockNum++;
			supInode->freeBlock[deleteInode->direct_addr[i]] = 1;
		}
	}

	/*cout << supInode->freeBlockNum << endl;
	cout << deleteInode->indirect_addr << endl;*/
	
	if (deleteInode->indirect_addr != -1)
	{
		cout << "\t\tDelete indirect address in file " << deleteInode->finode.name << endl;
		indirect_address* inde = readBlock_indirect(deleteInode->indirect_addr);
		supInode->freeBlockNum++;
		supInode->freeBlock[deleteInode->indirect_addr] = 1;
		/*cout << inde->size << endl;*/
		for (int i = 0; i < inde->size; i++)
		{
			supInode->freeBlockNum++;
			supInode->freeBlock[inde->addr[i]] = 1;
		}
	}
	cout << "\t\tfinish delete " << deleteInode->finode.name << endl;

	synchronization();
}

void delete_Dir_usage(inode* deleteInode)
{
	cout << "\t\tStart delete direction " << deleteInode->finode.name << endl;
	supInode->freeInode[deleteInode->inode_num] = 1;
	supInode->freeInodeNum++;

	if (deleteInode->direct_addr[0] == -1)
		return;

	for (int i = 0; i < 10; i++)
	{
		if (deleteInode->direct_addr[i] == -1)
			continue;
		else
		{
			supInode->freeBlockNum++;
			supInode->freeBlock[deleteInode->direct_addr[i]] = 1;
			dir* di = readBlock_toDir(deleteInode->direct_addr[i]);
			for (int j = 0; j < di->dirNum; j++)
			{
				inode * in = readInode(di->direct[j].inodeID);
				if (in->finode.dirSize == 0 && in->finode.fileSize != 0)
				{
					delete_File_usage(in);
				}
				else
				{
					delete_Dir_usage(in);
				}
			}
		}
	}

	synchronization();

	if (deleteInode->indirect_addr != -1)
	{
		indirect_address* inde = readBlock_indirect(deleteInode->indirect_addr);
		supInode->freeBlockNum++;
		supInode->freeBlock[deleteInode->indirect_addr] = 1;
		for (int i = 0; i < inde->size; i++)
		{
			supInode->freeBlockNum++;
			supInode->freeBlock[inde->addr[i]] = 1;
			dir* di = readBlock_toDir(inde->addr[i]);
			for (int j = 0; j < di->dirNum; j++)
			{
				inode * in = readInode(di->direct[j].inodeID);
				if (in->finode.dirSize == 0 && in->finode.fileSize != 0)
					delete_File_usage(in);
				else
				{
					delete_Dir_usage(in);
				}
			}
		}
	}
	cout << "\t\tfinish delete " << deleteInode->finode.name << endl;

	synchronization();
}

void deleteDirect(string fileName)
{
	vector<string>fileSets = spliteString(fileName, '/');
	inode *cur_inode;
	cout << "Current directory is:  " << currentString << endl;
	cout << "Delete processing start:" << endl;

	if (fileSets[0] == ".")
	{
		cur_inode = current;
	}
	else if (fileSets[0] == "root")
		cur_inode = root;
	else
		cur_inode = root;

	string deleteFileName = fileSets[fileSets.size() - 1];

	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);

	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size() - 1; i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;
		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);

			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}

		cur_inode = readInode(inode_addr);
	}
	cout << endl;

	cout << "\tFinding the file's location" << endl;
	int file_addr = -1;
	bool find = false;

	for (int j = 0; j < 10; j++)
	{
		int addr = cur_inode->direct_addr[j];
		if (addr == -1)
			continue;
		cur_direct = readBlock_toDir(addr);

		for (int n = 0; n < cur_direct->dirNum; n++)
		{
			if (cur_direct->direct[n].directName == deleteFileName)
			{
				file_addr = cur_direct->direct[n].inodeID;
				for (int inde = n; inde < cur_direct->dirNum; inde++)
				{
					cur_direct->direct[inde] = cur_direct->direct[inde + 1];
				}
				cur_direct->dirNum--;

				fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
					addr * BLOCKSIZE, SEEK_SET);
				fwrite(cur_direct, sizeof(dir), 1, virtualDisk);

				find = true;
				break;
			}
		}

	}

	if (find == false)
	{
		if (cur_inode->indirect_addr != -1)
		{
			indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
			for (int j = 0; j < addrs->size; j++)
			{
				int addr = addrs->addr[j];
				cur_direct = readBlock_toDir(addr);

				for (int n = 0; n < cur_direct->dirNum; n++)
				{
					if (cur_direct->direct[n].directName == deleteFileName)
					{
						file_addr = cur_direct->direct[n].inodeID;
						for (int inde = n; inde < cur_direct->dirNum; inde++)
						{
							cur_direct->direct[inde] = cur_direct->direct[inde + 1];
						}
						cur_direct->dirNum--;

						fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
							addr * BLOCKSIZE, SEEK_SET);
						fwrite(cur_direct, sizeof(dir), 1, virtualDisk);

						find = true;
						break;
					}
				}
			}
		}


	}
	if (!find)
	{
		cout << "no such direction!" << endl;
		return;
	}

	cout << "\tModify informations for directory." << endl;

	inode* deleteInode = readInode(file_addr);
	if (deleteInode == current)
	{
		cout << "error!: current direction can not be delete!" << endl;
	}

	inode* parentInode = deleteInode->parent;

	do
	{
		cout << "\tModify size of directory for " << parentInode->finode.name << endl;
		time_t timer;
		time(&timer);
		parentInode->finode.modifyTime = timer;
		parentInode->finode.dirSize -= deleteInode->finode.dirSize;

		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * parentInode->inode_num, SEEK_SET);
		fwrite(parentInode, sizeof(inode), 1, virtualDisk);
		parentInode = parentInode->parent;

	} while (parentInode->inode_num != root->inode_num);
	cout << "\tDelete dir " << deleteInode->finode.name << endl;

	delete_Dir_usage(deleteInode);
	cout << "finished" << endl;
	synchronization();

}

void changeDirect(string fileName)
{
	vector<string>fileSets = spliteString(fileName, '/');
	inode *cur_inode;
	string temp = "";
	cout << "Current directory is:  " << currentString << endl;

	if (fileSets[0] == ".")
	{
		cur_inode = current;
		temp = currentString;
	}
	else if (fileSets[0] == "root")
	{
		cur_inode = root;
		temp = "/root";
	}
	else
	{
		cur_inode = root;
		temp = "/root";
	}
	for (int i = 1; i < fileSets.size(); i++)
	{
		temp += "/" + fileSets[i];
	}

	cout << "Destination directory is:  " << temp << endl;
	cout << "Delete processing start:" << endl;

	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);
	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size(); i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;
		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);

			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}

		cur_inode = readInode(inode_addr);
		cout << "\tSearching inode: " << cur_inode->finode.name << endl;
	}
	cout << endl;

	current = cur_inode;
	currentString = temp;
	cout << "Current directory: " << currentString << endl;

	cout << "finishied" << endl;

}

void printFormally(file_info info)
{
	int pos = 20;
	cout << setiosflags(ios::left);
	cout << "\t" << setw(pos/2) << info.name;
	string c;
	if (info.dirSize == 0 && info.fileSize != 0)
		c = "File";
	else
		c = "Directory";
	cout << "\t" << setw(pos) << c;


	cout << "\t" << setw(pos/2) << to_string(c=="File"?info.fileSize:info.dirSize)+ "KB";
	cout << "\t" << setw(pos) << getTime(info.createTime);
	cout << "\t" << setw(pos) << getTime(info.modifyTime) << endl;
}

void dir1()
{
	cout << setiosflags(ios::left);
	cout << "\t" << setw(20) << "Current directory:" << setw(10) << currentString << endl;
	cout << "\t" << setw(20) << "Size:" << setw(10) << current->finode.dirSize<<"KB" << endl;
	cout << "\t" << setw(20) << "Creating time:" 
		<< setw(10) << getTime(current->finode.createTime) << endl;

	cout << "\t" << setw(20) << "Modified time:" 
		<< setw(10) << getTime(current->finode.modifyTime) << endl;

	cout << endl;
	if (readBlock_toDir(current->direct_addr[0])->dirNum == 0)
	{
		cout << "No directories or files in this directory!" << endl;
		return;
	}

	cout << "Sub-directory list:" << endl;
	int pos = 20;
	cout << setiosflags(ios::left);
	cout << "\t" << setw(pos / 2) << "File Name";
	string c = "Data type";
	cout << "\t" << setw(pos) << c;


	cout << "\t" << setw(pos / 2) << "Size";
	cout << "\t" << setw(pos) << "Create Time";
	cout << "\t" << setw(pos) << "Last Modified Time" << endl;

	for (int i = 0; i < 10; i++)
	{
		if (current->direct_addr[i] == -1)
			continue;
		dir *d = readBlock_toDir(current->direct_addr[i]);
		for (int j = 0; j < d->dirNum; j++)
		{
			file_info i = readInode(d->direct[j].inodeID)->finode;
			printFormally(i);
		}
	}

	if (current->indirect_addr != -1)
	{
		indirect_address* indi = readBlock_indirect(current->indirect_addr);
		for (int i = 0; i < indi->size; i++)
		{
			dir *d = readBlock_toDir(indi->addr[i]);
			for (int j = 0; j < d->dirNum; j++)
			{
				file_info i = readInode(d->direct[j].inodeID)->finode;
				printFormally(i);
			}
		}
	}
}

void cp_usage_(inode* file1, string fileName2)
{
	vector<string>fileSets = spliteString(fileName2, '/');
	cout << "Current directory is:  " << currentString << endl;
	cout << "Create copy start:" << endl;

	inode *cur_inode;
	int size = file1->finode.fileSize;
	if (fileSets[0] == ".")
	{
		cur_inode = current;
	}
	else if (fileSets[0] == "root")
		cur_inode = root;
	else
		cur_inode = root;

	cur_inode->finode.dirSize += size;
	cout << "dirSize:" << cur_inode->finode.dirSize << endl;
	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * cur_inode->inode_num, SEEK_SET);
	fwrite(cur_inode, sizeof(inode), 1, virtualDisk);

	string createFileName = fileSets[fileSets.size() - 1];

	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);
	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size() - 1; i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;

		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);


			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}
		cur_inode = readInode(inode_addr);
		cur_inode->finode.dirSize += size;
		time_t timer;
		time(&timer);
		cur_inode->finode.modifyTime = timer;


		cout << "dirSize:" << cur_inode->finode.dirSize << endl;
		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * inode_addr, SEEK_SET);
		fwrite(cur_inode, sizeof(inode), 1, virtualDisk);
	}

	cout << endl;

	cout << "\tModify informations for directory." << endl;
	int addr;

	bool created1 = false;


	for (int j = 0; j < 10; j++)
	{

		addr = cur_inode->direct_addr[j];
		if (addr == -1)
		{
			dir* d = new dir();
			cur_inode->direct_addr[j] = addBlock();
			d->index = cur_inode->direct_addr[j];
			fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
				cur_inode->direct_addr[j] * BLOCKSIZE, SEEK_SET);
			fwrite(d, sizeof(dir), 1, virtualDisk);
			addr = cur_inode->direct_addr[j];
		}
		cur_direct = readBlock_toDir(addr);
		if (cur_direct->dirNum == DIRNUM)
		{
			continue;
		}

		else
		{
			cur_direct->direct[cur_direct->dirNum].inodeID = addInode();
			for (int si = 0; si < createFileName.size(); si++)
			{
				cur_direct->direct[cur_direct->dirNum].directName[si] = createFileName[si];
			}
			cur_direct->dirNum++;
			created1 = true;
			break;
		}
	}


	if (cur_inode->indirect_addr != -1 && created1==false)
	{
		indirect_address* in = readBlock_indirect(cur_inode->indirect_addr);

		for (int j = 0; j < in->size; j++)
		{

			addr = in->addr[j];

			cur_direct = readBlock_toDir(addr);
			if (cur_direct->dirNum == DIRNUM)
			{
				continue;
			}

			else
			{
				cur_direct->direct[cur_direct->dirNum].inodeID = addInode();
				for (int si = 0; si < createFileName.size(); si++)
				{
					cur_direct->direct[cur_direct->dirNum].directName[si] = createFileName[si];
				}
				cur_direct->dirNum++;
				created1 = true;
				break;
			}
		}
	}


	fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
		addr * BLOCKSIZE, SEEK_SET);
	fwrite(cur_direct, sizeof(dir), 1, virtualDisk);

	cout << "\tCreating inode for new file." << endl;
	synchronization();
	inode* newInode = new inode();
	newInode->parent = cur_inode;
	newInode->finode.fileSize = size;
	newInode->inode_num = cur_direct->direct[cur_direct->dirNum - 1].inodeID;
	for (int si = 0; si < createFileName.size(); si++)
	{
		newInode->finode.name[si] = createFileName[si];
	}

	if (size <= 10)
	{
		cout << "\tCopying direct block" << endl;
		for (int i = 0; i < size; i++)
		{
			newInode->direct_addr[i] = addBlock();
			cpBlock(file1->direct_addr[i], newInode->direct_addr[i]);
		}
	}
	else
	{
		cout << "\tCopying direct block" << endl;
		for (int i = 0; i < 10; i++)
		{
			newInode->direct_addr[i] = addBlock();
			cpBlock(file1->direct_addr[i], newInode->direct_addr[i]);
		}
		newInode->indirect_addr = addBlock();

		indirect_address in;
		indirect_address* file1_indi = readBlock_indirect(file1->indirect_addr);


		in.index = newInode->indirect_addr;
		in.size = 0;

		cout << "\tCopying indirect block" << endl;
		for (int i = 0; i < size - 10; i++)
		{
			int indir_addres = addBlock();
			in.addr[i] = indir_addres;
			//cout << in.addr[i] << endl;
			in.size++;
			cpBlock(file1_indi->addr[i], in.addr[i]);
		}

		fseek(virtualDisk, SUPER_BLOCKS * 1024 + INODESIZE * INODESNUM +
			newInode->indirect_addr * BLOCKSIZE, SEEK_SET);
		fwrite(&in, sizeof(indirect_address), 1, virtualDisk);

	}
	cout << "\tSaving changes" << endl;


	synchronization();
	fseek(virtualDisk, SUPER_BLOCKS * BLOCKSIZE + INODESIZE * newInode->inode_num, SEEK_SET);
	fwrite(newInode, sizeof(inode), 1, virtualDisk);

	synchronization();
	cout << "Finish create processing: create " << createFileName << endl;
}

void cp(string fileName1, string fileName2)
{
	vector<string>fileSets1 = spliteString(fileName1, '/');
	
	inode *file1_inode;
	if (fileSets1[0] == ".")
	{
		file1_inode = current;
	}
	else if (fileSets1[0] == "root")
		file1_inode = root;
	else
		file1_inode = root;



	string createFileName = fileSets1[fileSets1.size() - 1];

	dir* cur_direct = readBlock_toDir(file1_inode->direct_addr[0]);
	cout << "\tLocating.";
	for (int i = 1; i < fileSets1.size(); i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;

		for (int j = 0; j < 10; j++)
		{
			int addr = file1_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);


			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets1[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (file1_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(file1_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets1[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}
		file1_inode = readInode(inode_addr);
	}

	cout << endl;

	cp_usage_(file1_inode, fileName2);

	synchronization();
	cout << "finished copy" << endl;
}

void sum()
{
	int pos = 20;
	int pos2 = 20;
	cout << setiosflags(ios::left);

	int c1 = SUPER_BLOCKS + INODESIZE * (INODESNUM - supInode->freeInodeNum) / BLOCKSIZE 
		+ 1 + BLOCKSNUM - supInode->freeBlockNum;
	int c2 = root->finode.dirSize;
	cout << "Total size of this file system: " << c1 << "KB" << endl;
	cout << "File size in this file system: " << c2 << "KB" << endl;
	cout << "File system structure size(overhead): " << c1 - c2 << "KB" << endl;
	cout << "Memory can be used: " << supInode->size / BLOCKSIZE - c1 << "KB (total)" << endl;
	cout << "Memory can be used: " << BLOCKSNUM  - c1 << "KB (files)" << endl;

	cout << endl << setw(pos2) << "List:" << setw(pos) << "Super Block";
	cout << setw(pos) << "Inodes";
	cout << setw(pos) << "Blocks" << endl;;

	cout << setw(pos2)<< "Free:"<< setw(pos) << 0;
	cout << setw(pos) << supInode->freeInodeNum;
	cout << setw(pos) << supInode->freeBlockNum << endl;
	
	cout << setw(pos2) << "Allocated:" << setw(pos) << SUPER_BLOCKS;
	cout << setw(pos) << INODESNUM - supInode->freeInodeNum;
	cout << setw(pos) << BLOCKSNUM - supInode->freeBlockNum << endl;
}

void cat(string fileName)
{
	vector<string>fileSets = spliteString(fileName, '/');
	inode *cur_inode;
	cout << "Current directory is:  " << currentString << endl;
	cout << "Delete processing start:" << endl;

	if (fileSets[0] == ".")
	{
		cur_inode = current;
	}
	else if (fileSets[0] == "root")
		cur_inode = root;
	else
		cur_inode = root;

	string deleteFileName = fileSets[fileSets.size() - 1];


	dir* cur_direct = readBlock_toDir(cur_inode->direct_addr[0]);



	cout << "\tLocating.";
	for (int i = 1; i < fileSets.size(); i++)
	{
		cout << ".";
		int inode_addr = -1;
		bool find = false;
		for (int j = 0; j < 10; j++)
		{
			int addr = cur_inode->direct_addr[j];
			if (addr == -1)
				continue;
			cur_direct = readBlock_toDir(addr);


			for (int n = 0; n < cur_direct->dirNum; n++)
			{
				if (cur_direct->direct[n].directName == fileSets[i])
				{
					inode_addr = cur_direct->direct[n].inodeID;
					find = true;
					break;
				}
			}
			if (find)
				break;
		}
		if (find == false)
		{
			if (cur_inode->indirect_addr != -1)
			{
				indirect_address* addrs = readBlock_indirect(cur_inode->indirect_addr);
				for (int j = 0; j < addrs->size; j++)
				{
					int addr = addrs->addr[j];
					cur_direct = readBlock_toDir(addr);

					for (int n = 0; n < cur_direct->dirNum; n++)
					{
						if (cur_direct->direct[n].directName == fileSets[i])
						{
							inode_addr = cur_direct->direct[n].inodeID;
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			}


		}
		if (!find)
		{
			cout << "no such direction!" << endl;
			return;
		}

		cur_inode = readInode(inode_addr);
	}
	cout << "\tthe file's location is founded" << endl;

	cout << "\tFile Name:" << cur_inode->finode.name << endl;
	cout << "\tFile Size:" << cur_inode->finode.fileSize << endl;
	cout << "\tFile text:" << endl;
	cout << "\t";
	int counter = 0;
	for (int i = 0; i < 10; i++)
	{
		if (cur_inode->direct_addr[i] == -1)
			continue;

		file_struct* file = readBlock_file(cur_inode->direct_addr[i]);
		counter++;
		for (int j = 0; j < FILESIZE; j++)
		{
			cout << file->c[j];
		}
	}


	if (cur_inode->indirect_addr != -1)
	{
		indirect_address* indi = readBlock_indirect(cur_inode->indirect_addr);
		for (int i = 0; i < indi->size; i++)
		{
			file_struct* file = readBlock_file(indi->addr[i]);
			counter++;
			cout << endl;
			cout <<"In block :" << counter << endl;
			for (int j = 0; j < FILESIZE; j++)
			{
				cout << file->c[j];
			}
		}
	}
	cout << endl;

	cout << "finished" << endl;
}

void help()
{
	cout << "Functions:" << endl << endl;
	cout << "createFile fileName S:"<< endl; 
	cout << "\tDescription: able to create a file with path(fileName) and size(S) int KB" << endl;
	cout << "\tExample: createFile /root/fileName 245" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tfileName: \tdata type: string \texample: /root/myfile \trange: " << endl;
	cout << "\t\tfileSize: \tdata type: integer \texample: 245 \trange: 0 - 255" << endl << endl;

	cout << "deleteFile fileName:" << endl;
	cout << "\tDescription: able to delete a file and release its memory" << endl;
	cout << "\tExample: deleteFile /root/myfile" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tfileName: \tdata type: string \texample: /root/myfile \trange: " << endl << endl;

	cout << "cat fileName:" << endl;
	cout << "\tDescription: Print out the file contents" << endl;
	cout << "\tExample: cat /root/myfile" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tfileName: \tdata type: string \texample: /root/myfile \trange: " << endl << endl;

	cout << "createDir fileName:" << endl;
	cout << "\tDescription: able to create a empty directory" << endl;
	cout << "\tExample: createDir /root/myfile" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tdirName: \tdata type: string \texample: /root/myfile \trange: " << endl << endl;

	cout << "deleteDir fileName:" << endl;
	cout << "\tDescription: able to delete a directory and its sub-directory" << endl;
	cout << "\tExample: deleteDir /root/myfile" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tdirName: \tdata type: string \texample: /root/myfile \trange: " << endl << endl;

	cout << "changeDir fileName:" << endl;
	cout << "\tDescription: able to change current work directory" << endl;
	cout << "\tExample: changeDir /root/myfile" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tdirName: \tdata type: string \texample: /root/myfile \trange: " << endl << endl;

	cout << "dir:" << endl;
	cout << "\tDescription: List all the files and sub - directories under current working directory" << endl;
	cout << "\tExample: dir" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tno parameter " << endl << endl;

	cout << "sum:" << endl;
	cout << "\tDescription: Display the usage of storage space" << endl;
	cout << "\tExample: sum" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tno parameter " << endl << endl;
	


	cout << "cp fileName1 fileName2:" << endl;
	cout << "\tDescription: able to copy fileName1 to fileName2, "
		<<"and fileName2 can't not exist in directory before this order" << endl;
	cout << "\tExample: cp /root/copyFile /root/targetFile:" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tfileName1: \tdata type: string \texample: /root/myfile \trange: " << endl;
	cout << "\t\tfileName2: \tdata type: string \texample: /root/myfile \trange: " << endl << endl;

	cout << "format:" << endl;
	cout << "\tDescription: formatting the file system" << endl;
	cout << "\tExample: format" << endl;
	cout << "\tParameter: " << endl;
	cout << "\t\tno parameter " << endl << endl;
}

void setOrder()
{
	string order;
	int i = 0;

	cout << "You can input :help: to get description of all the orders" << endl;
	cout << "[Current Directory: " << currentString << "/ ]#";
	while (getline(cin, order))
	{
		cout << endl;
		i++;
		if (order == "exit")
		{
			synchronization();
			fclose(virtualDisk);
			break;
		}
		vector<string>splite_string = spliteString(order,' ');

		string fileName;
		int fileSize;
		string fileName2;

		switch (test_regex_match1(order))
		{
		case 2:
			fileName = splite_string[1];
			fileSize = stoi(splite_string[2]);
			cout << "create file" << endl;
			createFile(fileName, fileSize);
			break;
		case 1:
			fileName = splite_string[1];
			cout << "delete file" << endl;
			deleteFile(fileName);
			break;
		case 3:
			fileName = splite_string[1];
			cout << "create direct" << endl;
			createDirect(fileName);
			break;
		case 4:
			fileName = splite_string[1];
			cout << "delete direct" << endl;
			deleteDirect(fileName);
			break;
		case 5:
			fileName = splite_string[1];
			cout << "Change directory" << endl;
			changeDirect(fileName);
			break;
		case 6:
			cout << "Current directory info:" << endl;
			dir1();
			break;
		case 7:
			cout << "Copy:>" << endl;
			fileName = splite_string[1];
			fileName2 = splite_string[2];
			cp(fileName, fileName2);
			break;
		case 8:
			cout << "Summation:>" << endl;
			sum();
			break;
		case 9:
			cout << "Text:>" << endl;
			fileName = splite_string[1];
			cat(fileName);
			break;
		case 10:
			cout << "formatting" << endl;
			formatting();
			break;
		case 11:
			cout << "Help" << endl;
			help();
			break;
		default:
			cout << "error: this order can't be executed" << endl;
			break;
		}
		cout << endl;
		cout << endl;
		cout << "[Current Directory: " << currentString << "/ ]#";

	
	}
} 