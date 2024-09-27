#include <iostream>
#include <vector>
#include <string>

size_t split(const std::string& str, const std::string& sep, std::vector<std::string>& result) {
    // new....music.#pop...
    size_t pos = 0, index = 0;
    while (index < str.size()) {
        pos = str.find(sep, index);
        if (pos == std::string::npos) {
            // 最后一次没有找到
            std::string tmp = str.substr(index);
            result.push_back(std::move(tmp));
            return result.size();
        }
        if (index == pos) {
            index = pos + sep.size();
            continue;
        }
        std::string tmp = str.substr(index, pos - index);
        result.push_back(std::move(tmp));
        index = pos + sep.size();
    }
    return result.size();
}

int main() {
    std::string str = "new....music.#..pop...";
    std::string sep = ".";
    std::vector<std::string> vs;
    split(str, sep, vs);
    for (auto& s : vs) {
        std::cout << s << std::endl;
    }
    return 0;
}