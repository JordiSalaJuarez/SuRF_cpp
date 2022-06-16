#include <fstream>
#include <vector>
using std::string;

int main() {
    std::ifstream file("../data/load_randint");
    std::vector<string> keys;
    std::string key;
    while (std::getline(file, key)) {
        keys.push_back(key);
    }
}