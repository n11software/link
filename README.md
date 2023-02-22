# Link
Link is a web library for C++ built with a focus of speed.

# Build and Install Link
Make sure you install CMake, GCC, Ninja, and OpenSSL.

>   ------------------------------------------------------------------------------
>
>   **NOTE**: *Ninja is optional as you can also use Make as your build system but
>   this is untested and Ninja is known to be a much faster build system.*
>
>   ------------------------------------------------------------------------------

```
$ mkdir build && cd build
$ cmake -GNinja ..
$ ninja && ninja install
```
