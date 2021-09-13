#include "../ArgParser.h"

int main(int argc, char** argv) {
    CLEAR_LOG
    ArgParser parser = ArgParser()
        .addArg("--help",ARG_TAG) // adding args
        .addArg("--say",ARG_SET,{"-s"})                     // ARG_SET = <arg-name> <value>     (string)
        .addArg("--no-log",ARG_TAG,{"--nl","-nl","--nol"}); // ARG_TAG = <arg-name>             (bool)
                                                            // this doesn't support arguments without name like the "echo" command
    ParsedArgs pargs = parser.parse(argv,argc);

    if(!pargs) {
        std::cout << "The parsing procces failed OR there are no args!\n";
    }
    
    if(pargs["--help"]) { // [ and ] for ARG_TAGs (return: bool)
        std::cout << "This is a testing area!\n";
    }
    if(pargs("--say") != "") { // ( and ) for ARG_SETs (return: string)
        std::cout << pargs("--say") << "\n";
    }
    if(pargs["--no-log"]) {
        logging::file = ""; // ArgParser.h has an built-in logging system, which can be disabled by setting the file to ""
    }
}