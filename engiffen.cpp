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
		cout << "Command: " << argv[0] << " requires a folder name, e.g. " << argv[0] << " folder_name " << endl;
		return 0;
	}
	stringstream foldername;
	foldername << argv[1];
	if(system(foldername.str().insert(0, "ls ").c_str()))
	{
		cout << "Could not find " << foldername.str() << ", Quitting..." << endl;
		return 0;
	}
	stringstream frames;
	frames << foldername.str() << "/*.gif";
	cout << frames.str().insert(0, "gifsicle ").append(" > ").append(foldername.str()).append(".gif");
	//system(frames.str().insert(0, "gifsicle ").append(" > ").append(foldername.str()).append(".gif").c_str());
}
