#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'
PURPLE='\033[0;35m'

# Handle different commands
case "$1" in
    "clean")
        echo -e "${YELLOW}Cleaning build directory...${NC}"
        rm -rf build/
        exit 0
        ;;
    "install")
        echo -e "${YELLOW}Installing library...${NC}"
        sudo install build/lib/liblink.so /usr/local/lib/
        sudo ldconfig
	sudo install -d /usr/local/include/link/
	sudo install Include/*.hpp /usr/local/include/link/
        echo -e "${GREEN}Library installed successfully!${NC}"
        exit 0
        ;;
    "test")
        if [ -z "$2" ]; then
            # Run all tests
            echo -e "${YELLOW}Running all tests...${NC}"
            export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib
            ./run/test_client
            ./run/test_server
        else
            case "$2" in
                "client")
                    echo -e "${YELLOW}Running client test...${NC}"
                    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib
                    ./run/test_client
                    ;;
                "server")
                    echo -e "${YELLOW}Running server test...${NC}"
                    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/build/lib
                    ./run/test_server
                    ;;
                *)
                    echo -e "${RED}Invalid test specified. Use 'client' or 'server'${NC}"
                    exit 1
                    ;;
            esac
        fi
        exit 0
        ;;
esac

# Create build directory if it doesn't exist
mkdir -p build
mkdir -p build/lib
mkdir -p build/test
mkdir -p run

# Count total steps (source files + library + test program)
total_files=$(find Source -name "*.cpp" | wc -l | tr -d ' ')
total_steps=$((total_files + 2))
current=1

# Status function
print_status() {
    printf "\r${PURPLE}[%d/%d]${YELLOW} %s${NC}                            " "$1" "$total_steps" "$2"
}

# Add compression libraries to linking
LIBS="-lssl -lcrypto -lz -lbz2 -lbrotlidec"

# Compile library source files
for source_file in Source/*.cpp; do
    filename=$(basename "$source_file")
    print_status $current "Compiling ${filename}..."
    g++ -c -fPIC \
        "$source_file" \
        -IInclude \
        -std=c++17 \
        -o "build/$(basename "$source_file" .cpp).o"
    ((current++))
done

# Create dynamic library
print_status $current "Creating dynamic library..."
g++ -shared \
    build/*.o \
    -o build/lib/liblink.so \
    $LIBS
((current++))

# Compile and client test program
print_status $current "[1/2] Building test client..."
g++ test/client_test.cpp \
    -o run/test_client \
    -Lbuild/lib \
    -IInclude \
    -llink \
    $LIBS \
    -pthread \
    -std=c++17

# Compile and server test program
print_status $current "[2/2] Building test server..."
g++ test/server_test.cpp \
    -o run/test_server \
    -Lbuild/lib \
    -IInclude \
    -llink \
    $LIBS \
    -pthread \
    -std=c++17

# Final status
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}Build successful!${NC}"
else
    echo -e "\n${RED}Build failed!${NC}"
    exit 1
fi

# Set library path
export LD_LIBRARY_PATH=build/lib:$LD_LIBRARY_PATH
