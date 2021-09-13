#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define ARG_SET 1
#define ARG_TAG 0

namespace logging {
    inline std::string file = "Debug.log";
}

#define CLEAR_LOG { if(logging::file != "") {std::ofstream out(logging::file,std::ios::trunc); out.close(); } }
#define LOG(message) { if(logging::file != "") { std::ofstream out(logging::file,std::ios::ate | std::ios::app); out << message << "\n"; out.close(); } }

struct Arg {
    ///
    /// type = true: set (takes 1 arg)
    /// type = false: tag (takes no arg)
    ///
    bool type = false;
    std::string name;
    std::vector<std::string> aliase;
        //internal
    bool is = false;
    std::string val = "";

    bool hasAlias(std::string name);
};

class ParsedArgs {
    std::vector<Arg> args_tag;
    std::vector<Arg> args_set;
    bool failed = false;
public:

    ParsedArgs(std::vector<Arg> args, bool failed) {
        if(!args.empty()) {
            for(auto& i : args) {
                if(i.type) {
                    args_set.push_back(i);
                }
                else {
                    args_tag.push_back(i);
                }
            }
        }

        this->failed = failed;
    }

    bool operator[](const char*);
    bool operator[](std::string arg);
    std::string operator()(std::string arg);
    operator bool();
};

class ArgParser {
    std::vector<Arg> args;

    size_t find(std::string name, bool& failed);
public:
    ArgParser& addArg(std::string name, bool type, std::vector<std::string> aliase = {});
    ParsedArgs parse(std::string args);
    ParsedArgs parse(char** args, int size);
};


#endif