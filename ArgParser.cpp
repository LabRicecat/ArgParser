#include "ArgParser.h"


bool Arg::hasAlias(std::string name) {
    for(auto& i : aliase) {
        if(i == name) {
            return true;
        }
    }
    return false;
}

bool ParsedArgs::operator[](const char* arg) {
    for(auto& i : args_tag) {
        if(i.name == arg || i.hasAlias(arg)) {
            return i.is;
        }
    }
    return false;
}

bool ParsedArgs::operator[](std::string arg) {
    for(auto& i : args_tag) {
        if(i.name == arg || i.hasAlias(arg)) {
            return i.is;
        }
    }
    return false;
}

std::string ParsedArgs::operator()(std::string arg) {
    LOG(args_set.size())
    for(auto& i : args_set) {
        LOG(i.name << " == " << arg)
        if(i.name == arg || i.hasAlias(arg)) {
            LOG("retuned i.val (" << i.val << ")")
            return i.val;
        }
    }
    return "";
}

ParsedArgs::operator bool() {
    return !failed;
}

size_t ArgParser::find(std::string name, bool& failed) {
    for(size_t i = 0; i < args.size(); ++i) {
        if(args[i].name == name || args[i].hasAlias(name)) {
            failed = false;
            return i;
        }
    }
    failed = true;
    return 0;
}

ArgParser& ArgParser::addArg(std::string name, bool type, std::vector<std::string> aliase) {
    args.push_back(Arg{type,name,aliase,false});
    return *this;
}

ParsedArgs ArgParser::parse(std::string args) {
    if(args == "") {
        return ParsedArgs({},true);
    }

    std::vector<std::string> tokens;
    std::string tmp;
    for(size_t i = 0; i < args.size(); ++i) {
        if(args[i] == ' ' && tmp != "") {
            tokens.push_back(tmp);
            tmp = "";   
        }
        else {
            tmp += args[i];
        }
    }
    if(tmp != "") {
        tokens.push_back(tmp);
    }
    for(size_t i = 0; i < tokens.size(); ++i) {
        LOG(tokens[i]);
    }
    bool failed = false;
    for(size_t i = 0; i < tokens.size(); ++i) {
        size_t index = find(tokens[i],failed);
        LOG("find(" << tokens[i] << ") = " << index)
        if(failed) {
            LOG("failed = true")
            return ParsedArgs({},true);
        }
        if(this->args[index].type) {
            if(i+1 >= tokens.size()) {
                LOG("i+1 >= tokens.size()")
                return ParsedArgs({},true);
            }

            this->args[index].val = tokens[i+1];
            LOG("this->args[" << index << "] = " << tokens[i+1])
            ++i;
        }
        else {
            this->args[index].is = true;
        }
    }

    auto ret = ParsedArgs(this->args,false);
    for(size_t i = 0; i < this->args.size(); ++i) { //clear
        this->args[i].val = "";
        this->args[i].is = false;
    }

    return ret;
}

ParsedArgs ArgParser::parse(char** args, int argc) {
    if(sizeof(args) == 1) {
        return ParsedArgs({},true);
    }
	std::string par;
	for(size_t i = 1; i < argc; ++i) {
		par += std::string(args[i]) + " ";
	}
	par.pop_back();
	return parse(par);
}