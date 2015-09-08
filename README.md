# libCom
## Introduction
Provides common communication and management features used by all subprojects of SecChat.

## What's included (by now)?
- wrapper around`std::unique_ptr` which securely erases the used memory on deletion
- simple binary protocol helper (convenience get-set access to members) based on compile-time code generation
- automatic OpenSSL compilation (cloned from official git repository) with custom flags deactivating unnecessary features

## Requirements
- CMake
- Compiler and linker platform supported by Cmake
- Python 3
- cogapp (python package)

### On Windows (for OpenSSL):
- mingw
- msys (bundled with perl)

## How to use
### Adding to project
#### Init git submodules:
```
$ git submodule add <repo url> <path>
```
```
$ git submodule update --init --recursive
```

#### CMake:
libCom
```
include_directories(<path-to-libcom>/include/)
```
```
add_subdirectory(<path-to-libcom>)
add_dependencies(YourProject libCom)
target_link_libraries(YourProject libCom)
```
<path-to-libcom> was supplied in submodules initialization

OpenSSL:
```
add_dependencies(YourProject openssl)
target_link_libraries(YourProject ssl crypto)
```

### Binary protocol
To add new mesages and their specifications, create a new file in `protocol/` describing the message (available data types and structures
can be found in `tool/generator.py`. After that, add the message specification file name to `protocol/protocol.messages`.
