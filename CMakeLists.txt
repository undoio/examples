# This CMake file builds the example programs in this directory.

cmake_minimum_required(VERSION 2.6)
project(examples C CXX)

INCLUDE(CheckIncludeFiles)

set(CMAKE_BUILD_TYPE Debug)

find_package(Threads)

CHECK_INCLUDE_FILES("libaio.h" HAVE_LIBAIO)
IF(HAVE_LIBAIO)
  find_library(LIBAIO NAMES aio)
ELSE()
  message(WARNING "libaio not found. The following examples cannot be compiled: aio.
Install 'libaio-dev' on deb-based distributions (Ubuntu, Debian, etc.), or 'libaio-devel' on rpm-based ones (Fedora, Red Hat, CentOS, etc.) to compile this example.")
ENDIF()

set(aio_maybe_exclude_from_all)
IF(NOT HAVE_LIBAIO)
   set(aio_maybe_exclude_from_all EXCLUDE_FROM_ALL)
ENDIF()
add_executable(aio ${aio_maybe_exclude_from_all} aio.c)
target_link_libraries(aio ${LIBAIO})

add_executable(cache cache.c)
target_link_libraries(cache m)

add_executable(cache-cpp cache-cpp.cpp)

file(MAKE_DIRECTORY cache-distributed)
add_executable(cache-distributed cache-distributed/cache-distributed.c)
set_target_properties(cache-distributed PROPERTIES RUNTIME_OUTPUT_DIRECTORY cache-distributed)
target_link_libraries(cache-distributed m)

add_executable(cpubound cpubound.cpp)

add_executable(deadlock deadlock.c)
target_link_libraries(deadlock ${CMAKE_THREAD_LIBS_INIT})

add_executable(hashtable hashtable.c)

add_executable(hello-world hello-world.c)

add_executable(linked-list linked-list.c)
target_link_libraries(linked-list ${CMAKE_THREAD_LIBS_INIT})

add_executable(malloc-var malloc-var.c)

add_executable(race race.cpp)
target_link_libraries(race ${CMAKE_THREAD_LIBS_INIT})

add_executable(simple simple.c)

add_executable(sine sine.c)
target_link_libraries(sine m)

add_executable(stacksmash stacksmash.c)

add_executable(threads threads.c)
target_link_libraries(threads ${CMAKE_THREAD_LIBS_INIT})

add_executable(workers workers.c)
target_link_libraries(workers ${CMAKE_THREAD_LIBS_INIT})

