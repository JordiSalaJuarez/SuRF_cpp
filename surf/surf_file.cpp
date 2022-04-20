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


auto main() -> int
{
	std::ifstream file("data/input");
	auto builder = yas::LoudsBuilder::from_stream(file);
	auto surf = yas::Surf::from_builder(builder, 1);
	std::ifstream same_file("data/input");
	for(auto key = std::string(); std::getline(same_file, key);){
		if (surf.look_up(key)){
			std::cout << "Query (surf.look_up(\""<< key <<"\") passed" << std::endl;
		} else {
			std::cout << "Query (surf.look_up(\""<< key <<"\") failed" << std::endl;
		}
	}
	auto wrong_keys = {"abc", "cba"};
	for(const auto &key: wrong_keys){
		if (surf.look_up(key)){
			std::cout << "Query (surf.look_up(\""<< key <<"\") failed" << std::endl;
		} else {
			std::cout << "Query (surf.look_up(\""<< key <<"\") passed" << std::endl;
		}
	}

	return 0;
}
