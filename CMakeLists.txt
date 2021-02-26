cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

## Find required libraries
set(CMAKE_PREFIX_PATH deps/PocoInstall)
find_package(Poco REQUIRED Foundation JSON CppParser)

include_directories(src
                    deps/skia
                    deps/v8
                    deps/PocoInstall/include)

link_directories(deps/skia/out/Shared)

## To suppress warning from Poco library (Clang only)
if(${CMAKE_CXX_COMPILER} MATCHES "clang")
    add_compile_options("-Wno-implicit-const-int-float-conversion")
endif()

add_compile_options("-Wall")
add_compile_definitions(COCOA_PROJECT=1 COCOA_USE_OPENCL=1 V8_COMPRESS_POINTERS)
add_executable(Cocoa
               src/Main.cc)

add_subdirectory(src/Core)
add_subdirectory(src/Ciallo)

target_link_libraries(Cocoa
        PRIVATE
            Core
            Ciallo)
