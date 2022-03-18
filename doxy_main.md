# Common macros and functions for C++
## Usage
Clone (as a submodule or otherwise):
```shell
git submodule add git@github.com:nvsl/cpp-common.git vendor/cpp-common
```

Add the directory as a include directory:
```shell
g++ -iquote"vendor/cpp-common/include" main.cc -o main
```

Use the files in your project:
```cpp
#include "nvsl/string.hh"
#include <iostream>

int main() {
    for (const auto tok : split("Hello! World.", " ")) {
        std::cout << tok << std::endl;
    }
}
```

Prints:
```
Hello!
World
```

## Available files
- [string.hh](@ref string.hh)
- [envvars.hh](@ref envvars.hh)
- [error.hh](@ref error.hh)
- [stats.hh](@ref stats.hh)

## Supported operations
Docs available at [https://nvsl.github.io/cpp-common/docs/html/](nvsl.io/cpp-common/docs/html).
