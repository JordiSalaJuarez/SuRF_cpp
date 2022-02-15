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
#include "utils.h"
#include "surf.h"

struct Louds {
	std::vector<char> labels;
	std::vector<bool> has_child;
	std::vector<bool> louds;
	
	size_t rank(size_t pos) __attribute__ ((pure)) {
		assert( pos < size(has_child));
		auto count = 0;
		for (auto i = 0 ; i <= pos; i++) 
			count += has_child[i];
		return count;
	}

	size_t select(size_t count) __attribute__ ((pure)){
		auto curr = 0;
		for (auto i = 0 ; i <= size(louds); i++){
			curr += louds[i];
			if (curr = count) return i;
		}
		return -1;
	}

	bool look_up(std::string word) {
		auto level = 0;
		auto begin = select(rank(0) + 1);
		auto end = select(rank(0) + 2);
		while(level < size(word)){
			size_t i = begin;
			for (; i < end; i++){
				if (labels[i] == word[level]) {
					auto begin = select(rank(i) + 1);
					auto end = select(rank(i) + 2);
					level++;
					break;
				}
			}
			if (i == end) return false;	
		}
		return true;
	}
};

int main()
{
	std::stringstream iss;
	iss << "aaa" << std::endl;
	iss << "aba" << std::endl;
	iss << "abb" << std::endl;
	iss << "abc" << std::endl;
	iss << "aca" << std::endl;
	iss << "acb" << std::endl;


	// auto file_path = "data/test.txt";
	// auto is_there = std::filesystem::exists(file_path);
	// std::cout << std::filesystem::current_path() << std::endl;
	// assert(is_there); // File not found
	// std::ifstream input(file_path);
	auto builder = LoudsBuilder::from_stream(iss);

	auto &labels = builder.labels;
	auto &has_child = builder.has_child;
	auto &louds = builder.louds;


	auto sparse = LoudsSparse::from_builder(builder);
	

	for (auto x : sparse.labels)
		std::cout << x;
	
	std::cout << "\n";
	for (auto x : sparse.has_child)
		std::cout << (x ? "1" : "0");
	std::cout << "\n";
	for (auto x : sparse.louds)
		std::cout << (x ? "1" : "0");
	std::cout << "\n";
	std::cout << "\nHello CMake." << std::endl;
	return 0;
}
