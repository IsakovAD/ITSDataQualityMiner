#pragma once

#include<iostream>
#include<unordered_map>
#include <algorithm>
#include<fstream>
#include <vector>
#include <string>





std::string trim(const std::string& str) {
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(), [](char c) {
        return c == ',' || c == '\"' || c == ' ';
    }), result.end());
    return result;
}


std::vector<std::unordered_map<std::string, std::string>> parseJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open JSON file "+ filename);
    }

    std::vector<std::unordered_map<std::string, std::string>> groups;
    std::unordered_map<std::string, std::string> current_group;

    std::string line;
    bool in_group = false;  // Track if we're inside a group

    while (getline(file, line)) {
        line = trim(line); //check if needed
        if (line.empty()) continue;

        if (line == "{") {
            in_group = true;
            current_group.clear();
            continue;
        }
        if (line == "}") {
            in_group = false;
            groups.push_back(current_group);
            continue;
        }

          auto colon_pos = line.find(':');
          if (colon_pos == std::string::npos) continue;

          std::string key = trim(line.substr(0, colon_pos));
          std::string value = trim(line.substr(colon_pos + 1));
          if (!key.empty()) {
                current_group[key] = value;
          }
    }


    // Handle single group not enclosed in braces
    if (!current_group.empty() && groups.empty()) {
        groups.push_back(current_group);
    }

    return groups;
}
