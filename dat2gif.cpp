#include<iostream>
#include<stdio.h>
#include<cstdlib>
#include<string>
#include<sstream>
using namespace std;

main(int argc, char* argv[])
{
	if(argc <2 )
	{
		cout << "Command: " << argv[0] << " requires a file name, e.g. " << argv[0] << " file_name number_of_particles" << endl;
		return 0;
	}
	int n = argv[2];
	stringstream filename;
	filename << argv[1];
	if(system(filename.str().insert(0, "ls ").c_str()))
	{
		cout << "Could not find " << filename.str() << ", Quitting..." << endl;
		return 0;
	}
	cout << "\n Done." << endl;
}
