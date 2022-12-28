# ArgParser v.1.0.3
A simple argument parser API for and in C++

Include the `ArgParser.h` file and don't forget to link the `ArgParser.cpp` file when compiling!

Create an new ArgParser using
```cpp
ArgParser parser;
```

With calling the `addArg()` member function, you can add new arguments. <br> <br>
The paramethers are:  <br>
`name` : the name of the argument (std::string) <br>
`type` : the type of the argument (ARG_GET,ARG_SET,ARG_TAG) <br>
`aliase` : the aliase of the argument (std::vector<std::string>) <br>
`priority` : either `Arg::Priority::FORCE` which means it isn't optional or `Arg::Priority::IGNORE` which means it isn't allowed to use this argument.

This sould look like this:
```cpp
ArgParser parser = ArgParser()
  .addArg(<name>,<type>,<aliase>,<priority>)
  .addArg(...)
  ...
  ;
 ```
There are 3 types of arguments: <br>
`ARG_GET` : takes the next unused string which doesn't start with isn't the name of an `ARG_SET` or `ARG_TAG` as input. Like `echo <get> ... ` <br>
`ARG_SET` : has to be called with `<name> <value>` and takes value as input <br>
`ARG_TAG` : boolean, takes no input, has to be called with `<name>` <br> <br>
The `-` or `--` have to be added to the name manually, else they can be called without them.

Here an example:
```cpp
#include "ArgParser.h"

int main(int argc, char** argv) {
  ArgParser parser = ArgParser()
    .addArg("--help",ARG_TAG,{"--h"})
    .addArg("-print",ARG_GET,{"-p"},Arg::Priority::FORCE)
    .addArg("-set",ARG_SET,{"-s"});
    
  ParsedArgs pargs = parser.parse(argv,argc);
  
  if(!pargs) {
    //error occured!
    std::cout << pargs.error() << "\n"; //prints the error message
    return -1;
    //you can check which error it is by using `pargs == ArgParserErrors::<error>`
  }

  if (pargs["--help"]) { // ARG_TAGs use `[]` (return bool)
     //...
     //print help
  }
  
  if(pargs("-set") != "") {
     std::cout << "SET:" << pargs("-set") << "\n"; // ARG_SETs use `()` (return std::string)
  }
  std::cout << pargs("-print") << "\n"; // ARG_GETs use `()` (return std::string)
}
```

Now, following call:
```
./programm -s _SET_ test
```
->
```
SET:_SET_
test
```
or
```
./programm test3 -s test2
```
->
```
SET:test2
test3
```

And so on.
