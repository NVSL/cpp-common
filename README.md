# Common macros and functions for C++
## Usage
Clone (as a submodule or otherwise):
```shell
git submodule add git@github.com:nvsl/cpp-common.git vendor/cpp-common
```

Add the directory as a include directory:
```shell
g++ -iquote"vendor/cpp-common/include"
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

## Supported operations
Docs available at [nvsl.io/cpp-common](nvsl.io/cpp-common).
