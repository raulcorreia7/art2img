cmake_minimum_required(VERSION 3.14)

file(READ "${CMAKE_CURRENT_LIST_DIR}/../CMakeLists.txt" _root_cmakelists)
string(REGEX MATCH "project\\([ \t\r\n]*art2img[ \t\r\n]+VERSION[ \t\r\n]+([0-9]+\\.[0-9]+\\.[0-9]+)" _ "${_root_cmakelists}")
if(NOT CMAKE_MATCH_1)
  message(FATAL_ERROR "Unable to determine project version from CMakeLists.txt")
endif()

message("${CMAKE_MATCH_1}")