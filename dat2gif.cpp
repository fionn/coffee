#include<iostream>
#include<stdio.h>
#include<cstdlib>
#include<string>
#include<sstream>
#include<fstream>
using namespace std;

main(int argc, char* argv[])
{
	if(argc <2 )
	{
		cout << "Command: " << argv[0] << " requires a file name, e.g. " << argv[0] << " file_name number_of_particles" << endl;
		return 0;
	}
	stringstream filename;
	string line;
	int n = atoi(argv[2]);

	filename << argv[1];
	if(system(filename.str().insert(0, "ls ").c_str()))
	{
		cout << "Could not find " << filename.str() << ", Quitting..." << endl;
		return 0;
	}

	ifstream input (filename.str().c_str()); 	
	for(int i=0; i<n; i++)
	{
		getline(input, line);
		cout << line << endl;
	}
	input.close();

	cout << "\nDone." << endl;
}
