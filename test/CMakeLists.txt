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

find_library(LIBPTHREAD pthread)
if (LIBPTHREAD)
    message(STATUS "Found libpthread: ${LIBPTHREAD}")
else()
    message(FATAL_ERROR "libpthread not found.")
endif()

link_directories(../build/hiredis 
                ../build/mosquitto/lib)
include_directories(../ 
                    ../src 
                    ../mosquitto/include)

set(SOURCES kx_test_filelist.c)

add_executable(rkxtest ${SOURCES})
target_link_libraries(rkxtest PRIVATE hiredis ${LIBEVENT_LIBRARY} ${LIBPTHREAD})

set(MQSOURCES mqtest.c)
add_executable(mqtest ${MQSOURCES})
target_link_libraries(mqtest PRIVATE :libmosquitto.so ${LIBPTHREAD})

set(MQPUBSUBSOURCES mqpubsub.c)
add_executable(mqpubsub ${MQPUBSUBSOURCES})
target_link_libraries(mqpubsub PRIVATE :libmosquitto.so ${LIBPTHREAD})