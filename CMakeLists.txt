cmake_minimum_required(VERSION 2.8)
project(stderred)

add_definitions(-std=c99)

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
if (HAVE_FWRITE_UNLOCKED)
  check_function_exists(fputs_unlocked HAVE_FPUTS_UNLOCKED)
  check_function_exists(fputc_unlocked HAVE_FPUTC_UNLOCKED)
  check_function_exists(fprintf_unlocked HAVE_FPRINTF_UNLOCKED)
endif (HAVE_FWRITE_UNLOCKED)
check_function_exists(error HAVE_ERROR)
check_function_exists(error_at_line HAVE_ERROR_AT_LINE)

if (NOT HAVE_GLIBC)
  check_function_exists(getprogname HAVE_GETPROGNAME)
endif (NOT HAVE_GLIBC)

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

set(stderred_sources stderred.c)

# Because of the way functions are overridden on mac they
# polyfill should be compiled into stderred instead of linked
if (APPLE)
  set(stderred_sources ${stderred_sources} polyfill.c)
endif (APPLE)

add_library(stderred SHARED ${stderred_sources})
add_library(test_stderred SHARED mocks.c ${stderred_sources})
add_executable(test_runner test.c)

add_library(polyfill SHARED polyfill.c)
target_link_libraries(test_runner dl polyfill)

if (NOT APPLE)
  target_link_libraries(stderred dl)
  target_link_libraries(test_stderred dl)
  set_property(TARGET stderred PROPERTY LINK_FLAGS "-init=init")
else (NOT APPLE)
  set_property(TARGET stderred PROPERTY LINK_FLAGS "-init _init")
endif(NOT APPLE)

install(TARGETS stderred DESTINATION lib)

include(CTest)
get_property(stderred_lib_location TARGET test_stderred PROPERTY LOCATION)
macro (do_test arg result)
  add_test (${arg} test_runner "${arg}")
  set_tests_properties(${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result})
  if (APPLE)
    set_tests_properties(${arg}
      PROPERTIES ENVIRONMENT "DYLD_INSERT_LIBRARIES=${stderred_lib_location}")
  else (APPLE)
    set_tests_properties(${arg}
      PROPERTIES ENVIRONMENT "LD_PRELOAD=${stderred_lib_location}")
  endif (APPLE)
endmacro (do_test)

do_test(printf "1 printf")
do_test(write ">2 write<")
do_test(fwrite ">2 fwrite<")
do_test(fwrite_unlocked "(>><)?>2 fwrite_unlocked<(><<)?")
do_test(fputc ">2< <= fputc")
do_test(fputc_unlocked "(>><)?>2<(><<)? <= fputc_unlocked")
do_test(fputs ">2 fputs<")
do_test(fputs_unlocked "(>><)?>2 fputs_unlocked<(><<)?")
do_test(fprintf ">2 fprintf<")
do_test(fprintf_unlocked "(>><)?>2 fprintf_unlocked<(><<)?")
do_test(vfprintf ">2 vfprintf<")
do_test(perror ">2 perror:[^\\n]+\\n<")
do_test(error ">[^:]*test_runner: (<>)?2 error[^\\n]+\\n<")
do_test(error_at_line ">[^:]*test_runner:[^:]+:[0-9]+: (<>)?2 error_at_line[^\\n]+\\n<")

