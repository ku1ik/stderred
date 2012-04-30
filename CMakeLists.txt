cmake_minimum_required(VERSION 2.8)
project(stderred)

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

if (!OSX)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
  else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (!OSX)

include(CheckIncludeFiles)
macro(CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)
  CHECK_INCLUDE_FILES("${USE_INCLUDES};${FILE}" ${VARIABLE})
  if (${VARIABLE})
    set(USE_INCLUDES ${USE_INCLUDES} ${FILE})
  endif (${VARIABLE})
endmacro(CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)

CHECK_INCLUDE_FILE_CONCAT(error.h HAVE_ERROR_H)

include(CheckFunctionExists)
check_function_exists(fwrite_unlocked HAVE_FWRITE_UNLOCKED)
check_function_exists(fputs_unlocked HAVE_FPUTS_UNLOCKED)
check_function_exists(fputc_unlocked HAVE_FPUTC_UNLOCKED)
check_function_exists(fwrite_unlocked HAVE_FWRITE_UNLOCKED)
check_function_exists(error HAVE_ERROR)
check_function_exists(error_at_line HAVE_ERROR_AT_LINE)

include(CheckSymbolExists)
check_symbol_exists(program_invocation_name "errno.h" HAVE_PROGRAM_INVOCATION_NAME)

add_library(stderred SHARED stderred.c)

install(TARGETS stderred DESTINATION lib)
