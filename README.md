# PSCHSL - PQCraft's Simple C HTTP Server Library

### Goals
- Simple
- Easy to use (well-designed and mostly self-explanatory API)
- Relatively fast and well-optimized

### Building
1. Install zlib-ng
2. `make -j` (see the Makefile for a list of the targets and options)
3. Optionally clean up intermediates with `make clean`

### Using
1. `make install`, or use `-I<repo dir>/src` and `-L<repo dir>`
2. `#include <pschsl/pschsl.h>`
3. `-lpschsl`

(See `src/pschsl/pschsl.h` for usage info)
