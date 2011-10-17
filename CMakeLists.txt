
cmake_minimum_required(VERSION 2.6)
add_definitions()

# Melinda library location, feel free to change 
set(MELINDA_DIR ../melinda)


add_library(melinda STATIC IMPORTED)
set_property(TARGET melinda PROPERTY IMPORTED_LOCATION ${MELINDA_DIR}/libmelinda.a)
include_directories(${MELINDA_DIR})

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")

set(CMAKE_CXX_FLAGS_PROF "-O3 -g -pg")
set(CMAKE_C_FLAGS_PROF "-O3 -g -pg")
set(CMAKE_EXE_LINKER_FLAGS_PROF "-pg")

#set default build type to Release
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

message(STATUS "BUILD : ${CMAKE_BUILD_TYPE}")

add_executable(plcm plcm.cpp Occurences.cpp Transactions.cpp Permutations.cpp)
target_link_libraries(plcm melinda pthread)

add_custom_target(plcm_doc doxygen doc)
