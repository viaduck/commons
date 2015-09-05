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

## How to use
### Adding to project
```
add_dependencies(YourProject libCom)
target_link_libraries(YourProject libCom)
```

### Binary protocol
To add new mesages and their specifications, create a new file in `protocol/` describing the message (available data types and structures
can be found in `tool/generator.py`. After that, add the message specification file name to `protocol/protocol.messages`.
