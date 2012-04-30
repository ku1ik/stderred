cmake_minimum_required(VERSION 2.8)
project(stderred)

if (!OSX)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
  else (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif (!OSX)

include(CheckCSourceCompiles)
check_c_source_compiles("
  #include <features.h>
  #ifndef __GLIBC__
  #error no gnu here
  #endif
  int main() {}
" HAVE_GLIBC
)

if (HAVE_GLIBC)
  # Make the actual program compilation use _GNU_SOURCE
  add_definitions(-D_GNU_SOURCE)
  # Make dependency checking have _GNU_SOURCE defined
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -D_GNU_SOURCE")
endif (HAVE_GLIBC)

include(CheckIncludeFiles)
check_include_files(error.h HAVE_ERROR_H)

include(CheckFunctionExists)
check_function_exists(fwrite_unlocked HAVE_FWRITE_UNLOCKED)
check_function_exists(fputs_unlocked HAVE_FPUTS_UNLOCKED)
check_function_exists(fputc_unlocked HAVE_FPUTC_UNLOCKED)
check_function_exists(fwrite_unlocked HAVE_FWRITE_UNLOCKED)
check_function_exists(error HAVE_ERROR)
check_function_exists(error_at_line HAVE_ERROR_AT_LINE)

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

add_library(stderred SHARED stderred.c)
target_link_libraries(stderred dl)

add_executable(test_runner test.c stderred)
target_link_libraries(test_runner dl)

install(TARGETS stderred DESTINATION lib)

include(CTest)
macro (do_test arg result)
  add_test (${arg} test_runner)
  set_tests_properties(${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
               ENVIRONMENT "LD_PRELOAD=")
endmacro (do_test)

do_test(printf "^1 printf\\n[^<]")
do_test(write ">2 write\\n<")
do_test(fprintf ">2 fprintf\\n<")
do_test(fwrite ">2 fwrite\\n<")
do_test(fputc ">2< <= fputc")
do_test(fputs ">2 fputs\\n<")
do_test(vfprintf ">2 vfprintf\\n<")
do_test(perror ">2 perror:[^\\n]+\\n<")

if (HAVE_ERROR)
  do_test(error ">[^:]*test_runner: 2 error[^\\n]+\\n<")
endif (HAVE_ERROR)

if (HAVE_ERROR_AT_LINE)
  do_test(error_at_line ">[^:]*test_runner:[^:]+:[0-9]+: 2 error_at_line[^\\n]+\\n<")
endif (HAVE_ERROR_AT_LINE)

