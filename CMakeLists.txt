##
## PLCM/Melinda
## (c) Benjamin Negrevergne, 2010
## Original LCM algorithm from Takaki Uno and Hiroki Arimura.
##
## The program was design and implemented in colaboration with
## Alexandre Termier and Jean-François Méhaut.
## 
## See the README file for installation, usage, and details.
## See the LICENSE file for licensing.

cmake_minimum_required(VERSION 2.6)
add_definitions()
include_directories(../../melinda)



set(CMAKE_C_FLAGS_DEBUG -g)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

project(plcm)

add_executable(plcm plcm.cpp Occurences.cpp Transactions.cpp Permutations.cpp)
add_library(melinda STATIC IMPORTED)
set_property(TARGET melinda PROPERTY IMPORTED_LOCATION ../../melinda/libmelinda.a)
target_link_libraries(plcm melinda pthread)


add_custom_target(cleane rm -rf *~)
add_custom_target(doc doxygen doc)
