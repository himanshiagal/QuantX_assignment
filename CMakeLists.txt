cmake_minimum_required(VERSION 3.27.0)
project(demo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_TOOLCHAIN_FILE "c:/Users/Dell/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_STANDARD 17)  # or your desired standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the 'include' directory to the search path for header files
include_directories(include)

# Add the third-party library directory (concurrentqueue) - full path to ensure it is found
include_directories("C:/Users/Dell/Downloads/test/external/concurrentqueue")

include(CTest)
enable_testing()

add_executable(demo src/main.cpp src/configLoader.cpp src/Logger.cpp)

# Additional compile options
target_compile_options(demo PRIVATE /bigobj)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
