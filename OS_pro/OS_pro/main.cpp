#include<stdio.h>
#include<stdlib.h>
#include<string>
#include"dataStructure.h"
#include"fileSystem.h"
#define NAME_LENGTH 100
#define BUFFER_SIZE 1<<20
using namespace std;


//创建文件
//char buffer[BUFFER_SIZE];
//int main() {
//	char name[NAME_LENGTH];
//	int sz = 0;
//	printf("Please input the name of the target file.\n");
//	scanf("%s", name);
//	printf("Please input the size(MB) of the target file.\n");
//	scanf("%d", &sz);
//	FILE *fout = fopen(name, "wb");
//	if (fout == NULL) {
//		printf("Cannot create the file.");
//		exit(-1);
//	}
//	printf("Wait...\n");
//	for (int i = 0; i < sz; i++) {
//		fwrite(buffer, BUFFER_SIZE, 1, fout);
//	}
//	fclose(fout);
//	printf("Finish.\n");
//	return 0;
//}


int main()
{
	//cout << sizeof(super) << endl;
	string data = "vm.dat";
	load((char*)data.c_str());


	system("pause");
}