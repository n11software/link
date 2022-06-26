rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSourceCode = $(call rwildcard,Source,*.cpp)
Objects = $(patsubst Source/%.cpp, Build/%.o, $(CPPSourceCode))
Directories = $(wildcard Source/*)

default: Library Install Test

Build/%.o: Source/%.cpp
	@mkdir -p $(@D)
	@g++ -c $^ -std=c++2a -fpic -pthread -o $@

Library: $(Objects)
	@g++ -shared -std=c++2a -pthread -o liblink.so $(Objects)

Install:
	@cp liblink.so /usr/lib/
	@cp -R Includes/* /usr/include/

.PHONY: Test

Test: Install
	@make -C Test run --no-print-directory