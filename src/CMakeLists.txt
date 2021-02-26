cmake_minimum_required(VERSION 3.0)
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
check_include_files(err.h HAVE_ERR_H)

include(CheckFunctionExists)
check_function_exists(fwrite_unlocked HAVE_FWRITE_UNLOCKED)
if (HAVE_FWRITE_UNLOCKED)
  check_function_exists(fputs_unlocked HAVE_FPUTS_UNLOCKED)
  check_function_exists(fputc_unlocked HAVE_FPUTC_UNLOCKED)
  check_function_exists(fprintf_unlocked HAVE_FPRINTF_UNLOCKED)
endif (HAVE_FWRITE_UNLOCKED)
check_function_exists(error HAVE_ERROR)
check_function_exists(error_at_line HAVE_ERROR_AT_LINE)

# BSD style error and warn functions
check_function_exists(err HAVE_ERR)
check_function_exists(verr HAVE_VERR)
check_function_exists(errc HAVE_ERRC)
check_function_exists(verrc HAVE_VERRC)
check_function_exists(errx HAVE_ERRX)
check_function_exists(verrx HAVE_VERRX)
check_function_exists(warn HAVE_WARN)
check_function_exists(vwarn HAVE_VWARN)
check_function_exists(warnc HAVE_WARNC)
check_function_exists(vwarnc HAVE_VWARNC)
check_function_exists(warnx HAVE_WARNX)
check_function_exists(vwarnx HAVE_VWARNX)
check_function_exists(err_set_file HAVE_ERR_SET_FILE)
check_function_exists(__fprintf_chk HAVE__FPRINTF_CHK)
check_function_exists(__write_nocancel HAVE__WRITE_NOCANCEL)

if (NOT HAVE_GLIBC)
  check_function_exists(getprogname HAVE_GETPROGNAME)
endif (NOT HAVE_GLIBC)

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

set(stderred_sources stderred.c)

# Because of the way functions are overridden on mac the
# polyfill should be compiled into stderred instead of linked
if (APPLE)
  set(stderred_sources ${stderred_sources} polyfill.c)
endif (APPLE)

add_library(stderred SHARED ${stderred_sources})
add_library(test_stderred SHARED mocks.c ${stderred_sources})
add_executable(test_runner test.c)

add_library(polyfill SHARED polyfill.c)
target_link_libraries(test_runner ${CMAKE_DL_LIBS} polyfill)

if (NOT APPLE)
  target_link_libraries(stderred ${CMAKE_DL_LIBS})
  target_link_libraries(test_stderred ${CMAKE_DL_LIBS})
endif(NOT APPLE)

install(TARGETS stderred DESTINATION lib)

include(CTest)
macro (do_test arg)
  add_test (NAME ${arg} COMMAND test_runner "${arg}")
  if (APPLE)
    set_tests_properties(${arg}
      PROPERTIES ENVIRONMENT "DYLD_INSERT_LIBRARIES=$<TARGET_FILE:test_stderred>")
  else (APPLE)
    set_tests_properties(${arg}
      PROPERTIES ENVIRONMENT "LD_PRELOAD=$<TARGET_FILE:test_stderred>")
  endif (APPLE)
endmacro (do_test)

macro (regex_test arg result)
  do_test(${arg})
  set_tests_properties(${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result})
endmacro (regex_test)


regex_test(printf "1 printf")
regex_test(write ">2 write<")
regex_test(fwrite ">2 fwrite<")
regex_test(fwrite_unlocked "(>><)?>2 fwrite_unlocked<(><<)?")
regex_test(fputc ">2< <= fputc")
regex_test(fputc_unlocked "(>><)?>2<(><<)? <= fputc_unlocked")
regex_test(fputs ">2 fputs<")
regex_test(fputs_unlocked "(>><)?>2 fputs_unlocked<(><<)?")
regex_test(fprintf ">2 fprintf<")
regex_test(fprintf_unlocked "(>><)?>2 fprintf_unlocked<(><<)?")
regex_test(vfprintf ">2 vfprintf<")
regex_test(perror ">2 perror:[^\\n]+\\n<")
regex_test(error ">[^:]*test_runner: (<>)?2 error[^\\n]+\\n<")
regex_test(error_at_line ">[^:]*test_runner:[^:]+:[0-9]+: (<>)?2 error_at_line[^\\n]+\\n<")
regex_test(err ">test_runner: (<>)?2 err(<>)?: [^\\n]+\\n<")
regex_test(err_empty ">test_runner: (<>)?2 err_empty(<>)?: [^\\n]+\\n<")
regex_test(verr ">test_runner: (<>)?2 verr(<>)?: [^\\n]+\\n<")
regex_test(errc ">test_runner: (<>)?2 errc(<>)?: [^\\n]+\\n<")
regex_test(errc_empty ">test_runner: (<>)?2 errc_empty(<>)?: [^\\n]+\\n<")
regex_test(verrc ">test_runner: (<>)?2 verrc(<>)?: [^\\n]+\\n<")
regex_test(errx ">test_runner: (<>)?2 errx(<>)?\\n<")
regex_test(errx_empty ">test_runner: (<>)?2 errx_empty(<>)?\\n<")
regex_test(verrx ">test_runner: (<>)?2 verrx(<>)?\\n<")
regex_test(warn ">test_runner: (<>)?2 warn(<>)?: [^\\n]+\\n<")
regex_test(warn_empty ">test_runner: (<>)?2 warn_empty(<>)?: [^\\n]+\\n<")
regex_test(vwarn ">test_runner: (<>)?2 vwarn(<>)?: [^\\n]+\\n<")
regex_test(warnc ">test_runner: (<>)?2 warnc(<>)?: [^\\n]+\\n<")
regex_test(warnc_empty ">test_runner: (<>)?2 warnc_empty(<>)?: [^\\n]+\\n<")
regex_test(vwarnc ">test_runner: (<>)?2 vwarnc(<>)?: [^\\n]+\\n<")
regex_test(warnx ">test_runner: (<>)?2 warnx(<>)?\\n<")
regex_test(warnx_empty ">test_runner: (<>)?2 warnx_empty(<>)?\\n<")
regex_test(vwarnx ">test_runner: (<>)?2 vwarnx(<>)?\\n<")
regex_test(err_uses_set_file "test_runner: 1 warnx\\n>>?test_runner: (<>)?2 warnx(<>)?\\n<")
do_test(blacklist test_runner "blacklist")

