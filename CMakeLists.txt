cmake_minimum_required(VERSION 3.16)

project(MyOSGuiPrototype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(X11 REQUIRED)

add_executable(myos-gui src/main.cpp)
target_link_libraries(myos-gui PRIVATE X11::X11)

target_compile_options(myos-gui PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -pedantic>
)
