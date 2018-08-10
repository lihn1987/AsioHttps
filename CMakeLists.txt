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
  ${DIR_SRC}
)
target_link_libraries(AsioHttps boost_system libboost_regex.a)
target_link_libraries(AsioHttps pthread)

