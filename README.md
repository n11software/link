# Link
Link is a web server library for C++ built with a focus of speed.

## Getting Link
You can find the Link repository at multiple places. These are all the current
repositories.

```
    GitHub (N11): https://github.com/n11software/link.git
                  git@github.com:n11software/link.git
    GitHub (aristonl): https://github.com/aristonl/link.git
                       git@github.com:aristonl/link.git
    repo.or.cz (aristonl): git://repo.or.cz/link.git
                           https://repo.or.cz/link.git
```

## Build and Install Link
Make sure you install CMake, GCC, Ninja, zlib, and OpenSSL version >=3.

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
>   **NOTE**: *By default, Link's install prefix is* `/usr/`*. You can change
>   change this by invoking something like* `cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -GNinja ..`
>   *which changes the install prefix to* `/usr/local`*. You may change this to your needs.*
>
>   ------------------------------------------------------------------------------

## Using Link
<!-- TODO: work on this -->
You can find a Link website template at [aristonl/linkproj](https://github.com/aristonl/linkproj). See its README for more information.

## License
Link is licensed under the BSD 3-Clause License. See LICENSE for more information.
