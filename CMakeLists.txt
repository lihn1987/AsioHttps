cmake_minimum_required(VERSION 3.5.1)
project(AsioHttps)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CUR_DIR .)
aux_source_directory(${CUR_DIR}/src DIR_SRC)
#SET(CMAKE_C_COMPILER gcc)
SET(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-std=c++14 -Wall")
#set(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
add_executable(AsioHttps
  ./src/main.cc
)
#SET(CMAKE_EXE_LINKER_FLAGS  "-lcrypto -lssl")
#set(CMAKE_CXX_LINK_EXECUTABLE "-lssl")
#这里由于另一个项目要用到ssl1.1.0的版本，而当前项目又需要用到ssl1.0的版本，由于没法设置版本，所以这里将就下
#target_link_libraries(AsioHttps boost_system libboost_regex.a ssl10 crypto10 dl z)
#target_link_libraries(AsioHttps pthread)
target_link_libraries(AsioHttps AsioHttpsLib)

ADD_LIBRARY(AsioHttpsLib STATIC
             ./src/AsioHttps.cc
             ./src/HttpInterface.cc)
target_link_libraries(AsioHttpsLib boost_system libboost_regex.a ssl10 crypto10 dl z)
target_link_libraries(AsioHttpsLib pthread)
