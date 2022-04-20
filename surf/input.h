#pragma once
#include <ios>
#include <string>
#include <fstream>
#include <istream>
#include <iterator>
#include <vector>
#include <iostream>

namespace input{

    struct File: public std::istream {
        File(std::stringstream &stream): std::istream(stream.rdbuf()){};
        File(const std::string &file_path): std::istream(std::ifstream(file_path, std::ifstream::in)){};
        std::string operator [] (size_t idx){
            std::string value;
            this->seekg(static_cast<std::streamoff>(idx));
            std::getline(*this, value);
            return value;
        }
    };
    auto begin(File& file){
        return std::istream_iterator<std::string>(file);
    }
    auto end(File&){
        return std::istream_iterator<std::string>();
    }
    struct Vector {
        std::vector<std::string> input;
        auto operator [] (size_t idx) const -> std::string const & {
            return input[idx];
        }
    };
    auto begin(Vector& vec){
        return std::begin(vec.input);
    }
    auto end(Vector& vec){
        return std::end(vec.input);
    }
        
}
