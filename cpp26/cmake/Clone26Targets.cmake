# clone26_add_target(<name> TEST|OBJECT SOURCES <files>... [REQUIRES <feature>...])
#
# TEST builds a GoogleTest executable and registers its tests with CTest. OBJECT only compiles its sources: the
# result is built and never run. REQUIRES lists feature names from Clone26Features.cmake (CLONE26_HAVE_<feature>).
# A target whose feature is missing is not created, and CMake says so with a "skipping" line.
function(clone26_add_target name kind)
  cmake_parse_arguments(PARSE_ARGV 2 ARG "" "" "SOURCES;REQUIRES")
  foreach(feature IN LISTS ARG_REQUIRES)
    if(NOT CLONE26_HAVE_${feature})
      message(STATUS "skipping ${name}: ${CLONE26_DESCRIPTION_${feature}} not supported by "
                     "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
      return()
    endif()
  endforeach()

  if(kind STREQUAL "TEST")
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE clone26 GTest::gtest_main)
    gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST)
  elseif(kind STREQUAL "OBJECT")
    add_library(${name} OBJECT ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE clone26)
  else()
    message(FATAL_ERROR "clone26_add_target: kind must be TEST or OBJECT, got '${kind}'")
  endif()
  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  message(STATUS "adding ${name}")
endfunction()
