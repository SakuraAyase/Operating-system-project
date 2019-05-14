
#include<string>
#include"dataStructure.h"
#include"fileSystem.h"

using namespace std;





int main()
{
	string data = "vm.dat";
	cout << "Welcome to FileSystem manaX" << endl << endl;

	cout << "Copy right @蒋怿" << endl;
	cout << "Student ID:201630588252" << endl;
	cout << "e-mail:moyishiyanyu@gmail.com" << endl;
	cout << "github:github.com/SakuraAyase" << endl << endl;

	//你记得写一下
	cout << "Copy right @蒋怿" << endl;
	cout << "Student ID:201630588252" << endl;
	cout << "e-mail:moyishiyanyu@gmail.com" << endl;
	cout << "github:github.com/SakuraAyase" << endl << endl;

	/*cout << sizeof(indirect_address) << endl;
	cout << sizeof(file_struct) << endl;*/
	initial((char*)data.c_str());
	load((char*)data.c_str());
	setOrder();
	

	system("pause");
}