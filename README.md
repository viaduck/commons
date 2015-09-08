# libCom
## Introduction
Provides common communication and management features used by all subprojects of SecChat.

## What's included (by now)?
- wrapper around`std::unique_ptr` which securely erases the used memory on deletion
- simple binary protocol helper (convenience get-set access to members) based on compile-time code generation

## Requirements
- CMake
- Compiler and linker platform supported by Cmake
- Python 3
- cogapp (python package)
### for openssl on windows:
- mingw, msys
- activestate perl

## How to use
### Adding to project
Init git submodules:
```
$ git submodule add <repo url> <path>
```
```
$ git submodule update --init --recursive
```

CMake:
```
include_directories(<path-to-libcom>/include/)
```
```
add_subdirectory(<path-to-libcom>)
add_dependencies(YourProject libCom)
target_link_libraries(YourProject libCom)
```
<path-to-libcom> was supplied in submodules initialization

### Binary protocol
To add new mesages and their specifications, create a new file in `protocol/` describing the message (available data types and structures
can be found in `tool/generator.py`. After that, add the message specification file name to `protocol/protocol.messages`.
