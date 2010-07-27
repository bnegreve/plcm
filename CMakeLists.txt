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

add_executable(plcm plcm.cpp Occurences.cpp Transactions.cpp Permutations.cpp)
target_link_libraries(plcm melinda pthread)

add_custom_target(plcm_doc doxygen doc)
