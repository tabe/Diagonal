find_path(ICU_INCLUDE_DIR unicode /usr/include /usr/local/include)

find_library(ICU_LIBRARY NAMES icui18n PATHS /usr/lib /usr/local/lib)

if(ICU_INCLUDE_DIR AND ICU_LIBRARY)
  set(ICU_LIBRARIES "${ICU_LIBRARY}")
  set(ICU_FOUND TRUE)
else(ICU_INCLUDE_DIR AND ICU_LIBRARY)
  set(ICU_FOUND FALSE)
endif(ICU_INCLUDE_DIR AND ICU_LIBRARY)

if(ICU_FOUND)
  execute_process(COMMAND icu-config --version
                  OUTPUT_VARIABLE ICU_VERSION_STRING
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "^([0-9]+)\\..*$" "\\1" ICU_VERSION_MAJOR "${ICU_VERSION_STRING}")
  string(REGEX REPLACE "^.*\\.([0-9]+)\\..*$" "\\1" ICU_VERSION_MINOR "${ICU_VERSION_STRING}")
  string(REGEX REPLACE "^.*\\.([0-9]+)$" "\\1" ICU_VERSION_PATCH "${ICU_VERSION_STRING}")
endif(ICU_FOUND)

if(ICU_FOUND)
  message(STATUS "ICU ${ICU_VERSION_STRING} found")
else(ICU_FOUND)
  message(STATUS "ICU not found")
endif(ICU_FOUND)
