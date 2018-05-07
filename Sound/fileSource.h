#pragma once
#include<iostream>
#include<fstream>
#include<vector>
#include<io.h>
#include<string>
using namespace std;


class fileSource
{
public:
	fileSource();
	virtual ~fileSource();
	vector<string> catalogInfo;
	void GetFiles(string path, string ext, vector<string> &files);//获取文件
};

