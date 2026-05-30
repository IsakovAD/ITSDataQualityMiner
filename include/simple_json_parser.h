#pragma once

#include<iostream>
#include<unordered_map>
#include <algorithm>
#include<fstream>

using namespace std;


string trim(const std::string& str) {
    string result = str;
    result.erase(std::remove_if(result.begin(), result.end(), [](char c) {
        return c == ',' || c == '\"' || c == ' ';
    }), result.end());
    return result;
}


vector<unordered_map<string, string>> parseJSON(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open JSON file "+ filename);
    }

    vector<unordered_map<string, string>> groups;
    unordered_map<string, string> current_group;

    string line;
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

          string key = trim(line.substr(0, colon_pos));
          string value = trim(line.substr(colon_pos + 1));
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
