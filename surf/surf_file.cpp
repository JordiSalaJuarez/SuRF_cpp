// surf_file.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <iterator>
#include <filesystem>
#include <cassert>
#include "surf.h"
using namespace std;
using namespace yas;
int main()
{

	ifstream file("data/input");
	auto builder = LoudsBuilder::from_stream(file);
	auto surf = Surf::from_builder(builder, 1);
	ifstream same_file("data/input");
	for(auto key = string(); getline(same_file, key);){
		if (surf.look_up(key)){
			cout << "Query (surf.look_up(\""<< key <<"\") passed" << endl;
		} else {
			cout << "Query (surf.look_up(\""<< key <<"\") failed" << endl;
		}
	}
	auto wrong_keys = {"abc", "cba"};
	for(auto &key: wrong_keys){
		if (surf.look_up(key)){
			cout << "Query (surf.look_up(\""<< key <<"\") failed" << endl;
		} else {
			cout << "Query (surf.look_up(\""<< key <<"\") passed" << endl;
		}
	}

	return 0;
}
