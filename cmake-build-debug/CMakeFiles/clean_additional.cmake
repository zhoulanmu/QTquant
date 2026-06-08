# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\QTQuant_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\QTQuant_autogen.dir\\ParseCache.txt"
  "QTQuant_autogen"
  )
endif()
