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
project(rkxtest)

link_directories(/home/yrb/src/hiredis/build 
                ../build/mosquitto/lib)
include_directories(/home/yrb/src 
                    ../src 
                    ../mosquitto/include)
# find_library(LIBHIREDIS_LIBRARY hiredis)
# if(LIBHIREDIS_LIBRARY)
#     message(STATUS "Found hiredis: ${LIBHIREDIS_LIBRARY}")
# else()
#     message(FATAL_ERROR "hiredis library not found.")
# endif()
set(SOURCES kx_test_filelist.c)

add_executable(rkxtest ${SOURCES})
target_link_libraries(rkxtest PRIVATE hiredis ${LIBEVENT_LIBRARY} ${LIBPTHREAD})
# find_package(Threads REQUIRED)
# target_link_libraries(rkx PRIVATE Threads::Threads)

set(MQSOURCES mqtest.c)
add_executable(mqtest ${MQSOURCES})
target_link_libraries(mqtest PRIVATE :libmosquitto.so.1)