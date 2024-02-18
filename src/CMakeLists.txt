#
# Copyright 2024-2024 yanruibinghxu@gmail.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
cmake_minimum_required(VERSION 3.12)
set(CMAKE_BUILD_TYPE Debug)
project(rkx)

find_library(LIBEVENT_LIBRARY event)
if(LIBEVENT_LIBRARY)
    message(STATUS "Found Libevent: ${LIBEVENT_LIBRARY}")
else()
    message(FATAL_ERROR "Libevent library not found.")
endif()

find_library(LIBPTHREAD pthread)
if (LIBPTHREAD)
    message(STATUS "Found libpthread: ${LIBPTHREAD}")
else()
    message(FATAL_ERROR "libpthread not found.")
endif()



link_directories(/home/yrb/src/hiredis/build 
                ../liblmdb 
                ../build/mosquitto/lib) 
include_directories(/home/yrb/src 
                    ../liblmdb 
                    ../mosquitto/include)
# find_library(LIBHIREDIS_LIBRARY hiredis)
# if(LIBHIREDIS_LIBRARY)
#     message(STATUS "Found hiredis: ${LIBHIREDIS_LIBRARY}")
# else()
#     message(FATAL_ERROR "hiredis library not found.")
# endif()

file(GLOB_RECURSE SOURCES "*.c" "*.h")

add_executable(rkx ${SOURCES})
# 添加依赖关系，确保在构建 rkx 之前构建 mosquitto
add_dependencies(rkx mosquitto)
target_link_libraries(rkx PRIVATE hiredis lmdb :libmosquitto.so ${LIBEVENT_LIBRARY} ${LIBPTHREAD})
# find_package(Threads REQUIRED)
# target_link_libraries(rkx PRIVATE Threads::Threads)