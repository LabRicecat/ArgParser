#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define ARG_GET 2
#define ARG_SET 1
#define ARG_TAG 0

struct Arg {
    enum class Priority {
        OPTIONAL,
        FORCE,
        IGNORE
    };
    ///
    /// type >= 2:  get     (takes 1 arg without call)
    /// type = 1:   set     (takes 1 arg with call)
    /// type = 0:   tag     (takes no arg with call)
    ///
    Priority priority = Priority::FORCE;
    int type = 0;
    std::string name;
    std::vector<std::string> aliase;
    // internal
    bool is = false;
    int fixed_pos = -1;
    std::string val = "";

    bool hasAlias(std::string name) {
        for(auto& i : aliase)
            if(i == name) return true;
        return false;
    }
};

enum class ArgParserErrors {
    OK,
    UNKNOWN,
    NO_ARGS,
    UNKNOWN_ARG,
    INVALID_SET,
    UNMATCHED_DEP_FORCE,
    UNMATCHED_DEP_IGNORE,
    POSITION_MISSMATCH
};

class ParsedArgs {
    std::vector<Arg> args_tag;
    std::vector<Arg> args_set;
    std::vector<Arg> args_get;
    ArgParserErrors error_code = ArgParserErrors::OK;
    std::string error_msg = "";

    std::vector<std::string> bin;
    bool bin_filled = false;
public:
    ParsedArgs(std::vector<Arg> args, ArgParserErrors error_code, std::string error_msg, std::vector<std::string> bin = {}) {
        if (!args.empty()) {
            for (auto &i : args) {
                if (i.type == ARG_SET)
                    args_set.push_back(i);
                else if (i.type == ARG_TAG)
                    args_tag.push_back(i);
                else
                    args_get.push_back(i);
            }
        }
        this->error_msg = error_msg;
        this->error_code = error_code;
        this->bin = bin;
        this->bin_filled = !bin.empty();
    }

    // returns args(tag)
    bool operator[](const char* arg) {
        for(auto& i : args_tag)
            if(i.name == arg || i.hasAlias(arg)) return i.is;
        return false;
    }
    bool operator[](std::string arg) {
        return operator[](arg.c_str());
    }

    // returns args(set and get)
    std::string operator()(std::string arg) {
        for(auto& i : args_set)
            if(i.name == arg || i.hasAlias(arg)) return i.val;
        for(auto& i : args_get) 
            if(i.name == arg || i.hasAlias(arg)) return i.val; 
        return "";
    }

    operator bool() { return error_code == ArgParserErrors::OK; }

    bool operator==(ArgParserErrors error) { return error_code == error; }
    bool operator!=(ArgParserErrors error) { return error_code != error; }

    // returns the error message 
    std::string error() const { return error_msg; }

    // returns true if `arg_name` is an argument that is set
    bool has(std::string arg_name) {
        for(auto i : this->args_get)
            if(i.name == arg_name && i.val != "") return true;

        for(auto i : this->args_set) 
            if(i.name == arg_name && i.val != "") return true;

        for(auto i : this->args_tag) 
            if(i.name == arg_name && i.is) return true;
        
        return false;
    }

    std::vector<std::string> get_bin() const { return bin; }

    bool has_bin() const { return bin_filled; }
};

class ArgParser {
    char strsym = ' ';
    uint32_t unusedGetArgs = 0;
    std::vector<Arg> args;
    bool has_bin = false;
    std::vector<std::string> bin;

    size_t find(std::string name, bool &failed) {
        for(size_t i = 0; i < args.size(); ++i)
            if(args[i].name == name || args[i].hasAlias(name)) {
                failed = false;
                return i;
            }
        failed = true;
        return 0;
    }

    size_t find_next_getarg(bool& failed) {
        if(unusedGetArgs == 0) {
            failed = true;
            return 0;
        }
        for(size_t i = 0; i < args.size(); ++i)
            if(args[i].type == ARG_GET && args[i].val == "") {
                --unusedGetArgs;
                failed = false;
                return i;
            }
        failed = true;
        return 0;
    }

public:
    ArgParser &addArg(std::string name, int type, std::vector<std::string> aliase = {},int fixed_pos = -1, Arg::Priority priority = Arg::Priority::OPTIONAL) {
        args.push_back(Arg{priority,type,name,aliase,false,fixed_pos});
        return *this;
    }
    ArgParser &enableString(char sym) {
        strsym = sym;
        return *this;
    }
    ArgParser &setbin() {
        has_bin = true;
        return *this;
    }
    ParsedArgs parse(char **argv, int argc) {
        if(argc == 1) {
            return ParsedArgs({},ArgParserErrors::NO_ARGS,"");
        }

        std::vector<std::string> par;
        for(size_t i = 1; i < argc; ++i) {
            par.push_back(argv[i]);
        }

        return parse(par);
    }
    ParsedArgs parse(std::vector<std::string> args) {
        if(args.size() == 0) return ParsedArgs({},ArgParserErrors::NO_ARGS,"");

        unusedGetArgs = 0;
        bool failed = false;
        for(size_t i = 0; i < this->args.size(); ++i)
            if(this->args[i].type == ARG_GET) ++unusedGetArgs;

        for(size_t i = 0; i < args.size(); ++i) {
            size_t index = find(args[i],failed);
            if(failed) {
                index = find_next_getarg(failed);
                if(failed) {
                    if(has_bin) {
                        for(size_t j = i; j < args.size(); ++j) bin.push_back(args[j]);
                        break;
                    }
                    return ParsedArgs({},ArgParserErrors::UNKNOWN_ARG,args[i]);
                }
                else if(this->args[index].type == ARG_GET) {
                    this->args[index].val = args[i];
                }
            }

            if(this->args[index].fixed_pos != -1 && this->args[index].fixed_pos != i) {
                return ParsedArgs({},ArgParserErrors::POSITION_MISSMATCH,args[i] + " not at right position!"); 
            }

            if(this->args[index].type == ARG_SET) {
                if(i+1 >= args.size()) {
                    return ParsedArgs({},ArgParserErrors::INVALID_SET,this->args[index].name);
                }

                this->args[index].val = args[i+1];
                ++i;
            }
            else if (this->args[index].type == ARG_TAG) {
                this->args[index].is = true;
            }
        }

        auto tmpa = this->args;
        unusedGetArgs = 0;
        for(size_t i = 0; i < this->args.size(); ++i) { //clear
            this->args[i].val = "";
            this->args[i].is = false;
            if(this->args[i].type == ARG_GET) {
                ++unusedGetArgs;
            }
        }

        // check for unmatching dependencies
        for(size_t i = 0; i < tmpa.size(); ++i) {
            if(((tmpa[i].val == "" && tmpa[i].type != ARG_TAG ) || (!tmpa[i].is && tmpa[i].type == ARG_TAG)) && tmpa[i].priority == Arg::Priority::FORCE) {
                return ParsedArgs(tmpa,ArgParserErrors::UNMATCHED_DEP_FORCE,tmpa[i].name);
            }
            else if(((tmpa[i].val != "" && tmpa[i].type != ARG_TAG ) || (tmpa[i].is && tmpa[i].type == ARG_TAG)) && tmpa[i].priority == Arg::Priority::IGNORE) {
                return ParsedArgs(tmpa,ArgParserErrors::UNMATCHED_DEP_IGNORE,tmpa[i].name);
            }
        }

        return ParsedArgs(tmpa,ArgParserErrors::OK,"",bin);
    }
};

#endif
