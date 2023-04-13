Beginners Guide to Link
=======================

>
> Author: Ariston Lorenzo <me@ariston.dev>
> <br>
> Last Modified: April 13, 2023
>

Introduction
------------
Link is a web server library built with a focus of speed for C++. Link is meant
to be a relatively easy framework to use and is considered the *ExpressJS* for
C++ developers.

A New Project (with linkproj)
-----------------------------
Once you have installed Link, you can clone the [aristonl/linkproj](https://github.com/aristonl/linkproj)
repository which is meant to be a simple template you can use to quickly setup
a Link-based project. To build, all you need to do is:

	mkdir build && cd build
	cmake ..
	make
	./website

Or you can also use ninja like so:

	mkdir build && cd build
	cmake -GNinja ..
	ninja
	./website

*linkproj* will run on port 3000 by default. You can start editing your site in
`www/` and handle all C++ related code in `src/`.

A New Project (from scratch)
----------------------------
Once you have installed Link, create a new folder where your website
will be stored. Create a new .cpp file in the root of the project (or create a separate
src/ directory). The following code is just a simple "Hello, World!" site.

```cpp
#include <Link.hpp>
#include <iostream>
#include <openssl/ssl.h>
#include <thread>

Link::Server server(8080) /* Set the port */

int main(int argc, char **argv)
{
	/* Handle requests for http://localhost:8080/ */
	server.Get("/", [](Link::Request* req, Link::Response* res) {
		/* Set the HTTP Header */
		res->SetHeader("Content-Type", "text/html; charset=UTF-8");
		/* Send the actual content of the page */
		res->SetBody("<h1>Hello, World!</h1>");
	});

	/* Start the server */
	std::cout << "Server started on port 8080." << '\n';
	server.Start();

	return 0;
}
```

To build the web server, you can either use Make (GNU or BSD) or CMake. The
following is a CMake example:

	cmake_minimum_required(VERSION 3.16)
	project(website C CXX)

	# Set include directory for OpenSSL on Mac
	include_directories(/opt/homebrew/opt/openssl@3/include)

	set(cpp_src main.cpp)

	add_executable(website ${cpp_src})
	target_compile_features(website PUBLIC cxx_std_17)
	set(CMAKE_CXX_FLAGS "-fpic -pthread")

	target_link_libraries(website PRIVATE link)

If you plan to use CMake, it is best to use it in tandem with Ninja. Here's a
Make example:

	PROG=website

	CXXFLAGS=-g -O2
	CXX=g++
	LIBS=-llink

	all: $(PROG)

	$(PROG): main.o
		$(CXX) -o $@ main.o $(LIBS)

	.cpp.o:
		$(CXX) $(CFLAGS) -c $*.cpp

	main.o: main.cpp

Once the `website` executable is built, run it using `./website` and open your
browser to http://localhost:8080 (or any other port you may have set it to). It should show "Hello, World!".

<br>

Obviously, we don't want to put all the HTML in the C++ file but instead have it
in a separate HTML file that we can call. To be able to do this, you can modify your source code like so:

```cpp
#include <fstream>

int main(int argc, char **argv)
{
	server.Get("/", [](Link::Request* req, Link::Response* res) {
		res->SetHeader("Content-Type", "text/html; charset=UTF-8");
		std::ifstream file("index.html");
		std::string content((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char());
		res->SetBody(content);				
	});

	std::cout << "Server started on port 8080." << '\n';
	server.Start();

	return 0;
}
```

Then you can create `index.html` and edit it like a normal HTML file.

> -----------------------------------------------------------------------------
>
> **NOTE**: *When calling index.html with `ifstream`, the location of index.html
> should be entered as the relative location from the executable. For example,
> if your file tree looks like this:*
>
> ```
>    .
>	|- bin
>	|  |- website
>	|- www
>	   |- index.html
> ```
>
> *The location of index.html should be filled in like so:*
> ```cpp
> std::ifstream file("../www/index.html");
> ```
>
> -----------------------------------------------------------------------------

You can also serve static files by pointing `server.SetStaticPages` to the folder
it's located in (usually `static/` or `public/`) like so:

```cpp
int main(int argc, char **argv)
{
	server.SetStaticPages("public/");

	/* rest of the code here... */
}
```

> -----------------------------------------------------------------------------
>
> **NOTE**: *The location of the folder must also be specified as its relative
> location to the executable.*
>
> -----------------------------------------------------------------------------

The files can be accessed with just `http://localhost:8080/` with the path
being just the directory structure of the `public/`
folder. For example, if this is your `public/`
folder structure:

	public
	|- css
	|  |- index.css
	|  |- fonts.css
	|- js
	   |- index.js

And you want to access `index.css`, the URL will be:
`http://localhost:8080/css/index.css`, and vice versa.
