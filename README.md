# libCom
## Introduction
Provides common communication and management features used by all subprojects of ViaDuck.

## What's included (by now)?
- Smart pointers:
    - `SecureUniquePtr`: Wrapper around`std::unique_ptr` which securely erases the used memory on deletion
    - `ValidPtr`: Pointer that tracks the state of encapsulated object
- Simple binary protocol helper (convenience get-set access to members) based on compile-time code generation
- `Bitfield`: Convenient bitfield manipulation functions
- `BlockCache`: Generation based fixed-cache
- `Buffer`: Dynamic raw buffers making use of secure memory erase
    - `String`: Secure strings based on buffer
- `KeyValueStorage`: Storage for key-value-pairs with (de)serialization
- `SecureStorage`: Secure RAM storage buffer, that prevents data tampering (only useful in conjunction with encryption methods exposed by core)
- `UTF8Decoder/UTF8Encoder`: UTF8 decoding and encoding with codepoint manipulation


## Requirements
- CMake 3.0
- Compiler and linker platform supported by CMake
- Python 3
- cogapp (python package)
- Google Test (referenced by submodule)
- openssl-cmake (referenced by submodule)

### Windows notes:
- Only mingw-w64 compiler infrastructure is supported (32 and 64 bits)

## How to use
### Adding to project
#### Init git submodule
```
$ git submodule add <repo url> <path>
```
```
$ git submodule update --init --recursive
```

#### CMake
```
include_directories(<path-to-libcom>/include/)
```
```
add_subdirectory(<path-to-libcom>)
target_link_libraries(YourProject libCom)
```
`<path-to-libcom>` was supplied in submodules initialization

### Binary protocol
To add new messages and their specifications, create a new file in `protocol/` (arbitrary subdirectory) describing the message (available data types and structures
can be found in examples (see `protocol/test/`). Message specification is automatically picked up by CMake (a CMake reload is necessary).
