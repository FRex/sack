# sack

Simple single header pure C89 library to take care of freeing many pieces of
memory at once using `free` plus some small convenience functions around it.

Written mainly for potential use with my small C utilities. You can find them
on my GitHub and arranged in a list here: [here](https://frex.github.io).

Named "sack" to invoke the image of putting all your buffers in a sack, tying
it up and throwing it into the lake to dispose of it.

To use `#include "sack.h"` and in a single source file that gets compiled
include it after doing `#define SACK_IMPLEMENTATION` to place implementations
in that file.
