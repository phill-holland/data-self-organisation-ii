cmake_minimum_required(VERSION 3.20)

set(DPCPP_HOME "/home/phill/sycl_workspace")
set(CMAKE_CXX_COMPILER "${DPCPP_HOME}/llvm/build/bin/clang++")

project(data-self-organisation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -gdwarf-4")

set(THREADS_PREFER_PTHREAD_FLAG ON)

set(LLVM_ENABLE_THREADS TRUE)
set(LLVM_PARALLEL_COMPILE_JOBS, 8)
set(LLVM_PARALLEL_LINK_JOBS, 8)

find_package(oneDPL REQUIRED)
find_package(Threads REQUIRED)

include_directories("${DPCPP_HOME}/llvm/build/include")
include_directories("${DPCPP_HOME}/llvm/build/include/sycl")
include_directories("/usr/include")
include_directories("src/include")

add_subdirectory(src)
add_subdirectory(test)

file(GLOB_RECURSE SOURCE CONFIGURE_DEPENDS 
"${PROJECT_SOURCE_DIR}/src/source/*.cpp" 
)