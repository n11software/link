Beginners Guide to Link
=======================

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
> **TODO**: *It's probably best to get linkproj working beforehand so we can use it as a basis for this section*
