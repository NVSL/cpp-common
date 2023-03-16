[![CircleCI](https://circleci.com/gh/NVSL/cpp-common/tree/master.svg?style=svg&circle-token=303fcb99481c32f94d3eac4fd719a1ae524efc10)](https://circleci.com/gh/NVSL/cpp-common/tree/master)

-   [Common macros and functions for
    C++](#common-macros-and-functions-for-c)
    -   [Usage](#usage)
        -   [PMemOps: CLWB, CLFLUSHOPT, CLFLUSH and
            MSYNC](#pmemops-clwb-clflushopt-clflush-and-msync)
    -   [String operations](#string-operations)
        -   [split()](#split)
        -   [zip(): join vector of string using a
            token](#zip-join-vector-of-string-using-a-token)
        -   [S(): String constructor](#s-string-constructor)
    -   [Clock](#clock)
        -   [Simple timing benchmark](#simple-timing-benchmark)
    -   [Available files](#available-files)
    -   [Supported operations](#supported-operations)

# Common macros and functions for C++

Header only collection of common macro functionality for C++ 20.

## Usage
Clone (as a submodule or otherwise):
```shell
git submodule add git@github.com:nvsl/cpp-common.git vendor/cpp-common
```

Add the directory as a include directory:
```shell
g++ -iquote"vendor/cpp-common/include" main.cc -o main
```


### PMemOps: CLWB, CLFLUSHOPT, CLFLUSH and MSYNC

PMemOps provides convinient functions to flush and drain arbitrary sized address
range.

#### Flush and drain using pmemops

```cpp
#include "nvsl/pmemops.hh"

int main() {
    nvsl::PmemOps *pmemops = new nvsl::PmemOpsClwb();
    
    /* uint64_t *val = pmalloc(sizeof(uint64_t)); */
    
    // CLWB
    pmemops->flush(val, sizeof(*val));
    
    // SFENCE
    pmemops->drain();
}

```

#### Streaming writes using pmemops
PMemOps supports streaming writes (NT stores) to perform memcpy.  
`streaming_wr()` automatically selects the right instruction depending on the
number of bytes being copied.


```cpp
#include "nvsl/pmemops.hh"

int main() {
    nvsl::PmemOps *pmemops = new nvsl::PmemOpsClwb();
    
    /* 
     * uint64_t *src = pmalloc(100 * sizeof(uint64_t)); 
     * uint64_t *dst = pmalloc(100 * sizeof(uint64_t)); 
     */
    
    // NT stores to do memcpy
    pmemops->streaming_wr(dst, src, 100*sizeof(uint64_t));
}
```

## String operations
Use the files in your project:

```
#include "nvsl/string.hh"
```

### split() 
```cpp
for (const auto tok : split("Hello! World.", " ")) {
    std::cout << tok << std::endl;
}
```

Prints:
```
Hello!
World
```

### zip(): join vector of string using a token
```cpp
std::cout << zip({"1", "2", "3"}, ", ")) << std::endl;
```

Prints:
```
1, 2, 3
```

### S(): String constructor
Convert `char*`, numbers or floats to `std::string`:

```cpp
using namespace nvsl;
char *buf = "some string";

std::string buf_str = S(buf);
std::string num = S(1);
```

## Clock
### Simple timing benchmark

```cpp
void do_something() {
    nvsl::Clock clk;

    clk.tick();
    sleep(1);
    clk.tock();
    
    clk.reconcile();
    
    std::cout << "Execution summary" << clk.summarize() << std::endl;
}
```

## Some of the utilities also have a C interface
```c
#include "nvsl/c-common.h"
```

### Available functions
```c
void nvsl_fatal(const char *fmt, ...);
int nvsl_is_log_enabled(int check_lvl);
int nvsl_log(const int lvl, const char *fmt, ...);
```

## Available files
- [clock.hh](include/nvsl/clock.hh)
- [envvars.hh](include/nvsl/envvars.hh)
- [error.hh](include/nvsl/error.hh)
- [pmemops.hh](include/nvsl/pmemops.hh)
- [stats.hh](include/nvsl/stats.hh)
- [string.hh](include/nvsl/string.hh)

## Supported operations
Docs available at [https://nvsl.github.io/cpp-common/](nvsl.io/cpp-common/).
