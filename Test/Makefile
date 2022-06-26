rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CPPSourceCode = $(call rwildcard,Source,*.cpp)
Objects = $(patsubst Source/%.cpp, Build/%.o, $(CPPSourceCode))
Directories = $(wildcard Source/*)

Build/%.o: Source/%.cpp
	@mkdir -p $(@D)
	@g++ -c $^ -std=c++2a -o $@

Test: $(Objects)
	@g++ $(Objects) -std=c++2a -llink -pthread -o Test

run: Test
	@./Test