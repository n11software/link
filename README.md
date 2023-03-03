# Link
Link is a web server library for C++ built with a focus of speed.

## Build and Install Link
Make sure you install CMake, GCC, Ninja, zlib, and OpenSSL.

>   ------------------------------------------------------------------------------
>
>   **NOTE**: *Ninja is optional as you can also use Make as your build system but
>   this is untested and Ninja is known to be a much faster build system.*
>
>   ------------------------------------------------------------------------------

```
$ mkdir build && cd build
$ cmake -GNinja ..
$ ninja && sudo ninja install
```
>   ------------------------------------------------------------------------------
>
>   **NOTE**: *By default, CMake's install prefix is* `/usr/local`*. You can change
>   change this by invoking something like* `cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -GNinja ..`
>   *which changes the install prefix to* `/usr`*. You may change this to your needs.*
>
>   ------------------------------------------------------------------------------

## License
Link is licensed under the BSD 3-Clause License. See LICENSE for more information.
