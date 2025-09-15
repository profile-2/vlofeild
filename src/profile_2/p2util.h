#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//#include <algorithm>

#ifndef P2_UTIL
#define P2_UTIL

namespace p2util{
    template<typename T>
    void Echo(const T& value, bool newLine = true) {
        std::cout << value;
        if (newLine) std::cout << std::endl;
    }

    std::string detectEncoding(std::ifstream& file) {
        char bom[3] = {0};
        file.read(bom, 3);
        size_t bytesRead = file.gcount();
        file.seekg(0, std::ios::beg);

        if (bytesRead >= 2 && bom[0] == '\xFF' && bom[1] == '\xFE') {
            return "UTF-16-LE";
        } else if (bytesRead >= 2 && bom[0] == '\xFE' && bom[1] == '\xFF') {
            return "UTF-16-BE";
        } else if (bytesRead >= 3 && bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF') {
            return "UTF-8";
        }
        return "ANSI";
    }

    std::string TxtToString(const std::string& filename, int maxCharacters = 100000) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        std::string encoding = detectEncoding(file);
        if (encoding == "UTF-16-LE" || encoding == "UTF-16-BE") {
            file.close();
            return "";
        }

        if (encoding == "UTF-8") {
            file.seekg(3, std::ios::beg);
        }

        std::string line;
        std::string fileString = "";
        while (std::getline(file, line) && fileString.size() < maxCharacters ){
            fileString.append(line);
        }
        

        file.close();
        return fileString;
    }
    
    std::vector<int> ValuesFromString(const std::string &s){
		int currentValue = 0;
		std::vector<int> values = {};
		for(auto &c : s){
			if(c == '\r' || c==','){
				values.push_back(currentValue);
				currentValue = 0;
			}
			else{
				currentValue *= 10;
				currentValue += c -'0';
			}
		}
		return values;
	}

    std::vector<int> ValuesFromString2(const std::string s){
		int currentValue = 0;
		std::vector<int> values = {};
		for(auto &c : s){
			if(c == '\r' || c=='\n' || c==',' ){
				values.push_back(currentValue);
				currentValue = 0;
			}
			else{
				currentValue *= 10;
				currentValue += c -'0';
			}
		}
		return values;
	}

    std::vector<int> ValuesFromCSV(const std::string& filename, int maxCharacters = 100000){
        return ValuesFromString2(TxtToString(filename, maxCharacters));
    }
}

#endif
