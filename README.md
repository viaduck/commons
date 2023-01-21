# Commons
Provides common features used by multiple subprojects of ViaDuck.

## Overview
### Base module
- Compile-time code generation of enums, protocol classes and sqlite classes.
  - `Bitfield`: Convenient bitfield manipulation functions (used in protocol classes)
  - `ConstexprString`: Compile-time string with concat support (used to generate sqlite queries)
- Custom logging infrastructure with various levels and outputs
- `ValidPtr`: Pointer that tracks the state of an encapsulated object

### Curve25519 module
- Adapted `curve25519` implementation for `OpenSSL` from `BoringSSL`

### Network module
- Platform-independent `Connection` class with support for `SSL`,
`CertificateStorage` for pinning, `SSLContext` for session resumption
- Convenience methods for exact reading/writing, (de)serializing protocol classes

## Requirements
- Compiler with C++ 14 support
- Python 3.4+ with `cogapp` package
- For Windows: `mingw-w64` infrastructure (32 or 64 bits)

## How to use
### Adding to project
1. Add as git submodule `external/commons`
2. In CMakeLists: `add_subdirectory(external/commons)` and link against `commons`

Note: In order to build Commons with only the base module, set `COMMONS_BASE_ONLY=ON`.

### Usage
- Add own enums, protocol or sqlite classes as definitions to `gen/` subdirectories
- See [tests](test) for usage examples

## Licensing
This library is subject to the GNU Lesser General Public License v3.0 (GNU
LGPLv3).

```
Copyright (C) 2015-2023  The ViaDuck Project

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
```
