# clone26_add_target(<name> TEST|OBJECT SOURCES <files>... [REQUIRES <feature>...] [LINK <targets>...]
#                    [OPTIONS <flags>...] [MUTANT [MUTANT_INCLUDE_DIR <dir>] [MUTANT_SOURCES <files>...]])
#
# TEST builds a GoogleTest executable and registers its tests with CTest. OBJECT only compiles its sources: the
# result is built and never run. REQUIRES lists feature names from Clone26Features.cmake (CLONE26_HAVE_<feature>).
# A target whose feature is missing is not created. LINK names the targets the new one links besides the library
# of the headers.
#
# MUTANT adds a twin, <name>_mutant, with the same sources, that finds a mutant header of Clone26Mutant.cmake before
# the real one. The twin is left out of the default build and of CTest: check_nets.sh builds it and expects it to
# fail. The twin uses the Tile mutant unless MUTANT_INCLUDE_DIR names another. With MUTANT_SOURCES, only those
# sources see the mutant and the others see the real header, which is how two translation units of one executable
# come to see different headers. The mutant sources are then compiled as an object library of their own.
function(clone26_add_target name kind)
  cmake_parse_arguments(PARSE_ARGV 2 ARG "MUTANT" "MUTANT_INCLUDE_DIR" "SOURCES;REQUIRES;LINK;OPTIONS;MUTANT_SOURCES")
  foreach(feature IN LISTS ARG_REQUIRES)
    if(NOT CLONE26_HAVE_${feature})
      message(STATUS "skipping ${name}: ${CLONE26_DESCRIPTION_${feature}} not supported by "
                     "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
      return()
    endif()
  endforeach()

  clone26_define_target(${name} ${kind} "${ARG_SOURCES}" "${ARG_LINK}" "${ARG_OPTIONS}" "")
  message(STATUS "adding ${name}")
  if(NOT ARG_MUTANT)
    return()
  endif()

  set(mutant_dir ${CLONE26_MUTANT_INCLUDE_DIR})
  if(ARG_MUTANT_INCLUDE_DIR)
    set(mutant_dir ${ARG_MUTANT_INCLUDE_DIR})
  endif()

  if(NOT ARG_MUTANT_SOURCES)
    clone26_define_target(${name}_mutant ${kind} "${ARG_SOURCES}" "${ARG_LINK}" "${ARG_OPTIONS}" "${mutant_dir}")
    return()
  endif()

  # Only some sources see the mutant. They are compiled as an object library with the mutant first on its include
  # path, and the executable of the twin links those objects with the sources that see the real header.
  set(real_sources ${ARG_SOURCES})
  list(REMOVE_ITEM real_sources ${ARG_MUTANT_SOURCES})
  clone26_define_target(${name}_mutant_objects OBJECT "${ARG_MUTANT_SOURCES}" "${ARG_LINK}" "${ARG_OPTIONS}"
                        "${mutant_dir}")
  clone26_define_target(${name}_mutant ${kind} "${real_sources};$<TARGET_OBJECTS:${name}_mutant_objects>"
                        "${ARG_LINK}" "${ARG_OPTIONS}" "${mutant_dir}" NO_MUTANT_INCLUDE)
endfunction()

# mutant_dir is empty for the real target. A target with a mutant_dir is a twin: it is left out of the default build
# and of CTest, and it finds mutant_dir first on its include path, unless NO_MUTANT_INCLUDE says that its own sources
# see the real header and only the objects it links see the mutant.
function(clone26_define_target name kind sources link options mutant_dir)
  set(exclude "")
  if(mutant_dir)
    set(exclude EXCLUDE_FROM_ALL)
  endif()

  if(kind STREQUAL "TEST")
    add_executable(${name} ${exclude} ${sources})
    target_link_libraries(${name} PRIVATE ${link} clone26 GTest::gtest_main)
    if(NOT mutant_dir)
      gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST)
    endif()
  elseif(kind STREQUAL "OBJECT")
    add_library(${name} OBJECT ${exclude} ${sources})
    target_link_libraries(${name} PRIVATE ${link} clone26)
  else()
    message(FATAL_ERROR "clone26_add_target: kind must be TEST or OBJECT, got '${kind}'")
  endif()

  target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  if(mutant_dir AND NOT "NO_MUTANT_INCLUDE" IN_LIST ARGN)
    target_include_directories(${name} BEFORE PRIVATE ${mutant_dir})
  endif()
  target_compile_options(${name} PRIVATE ${options})
  clone26_record_product(${name} ${kind})
endfunction()

# Writes what the target produces into <build dir>/products/, at generate time, so that check_nets.sh need not know
# where the generator puts it: the path of the executable of a TEST target, or the object files of an OBJECT target.
# Where the generator has several configurations (Ninja Multi-Config), the files go in a directory per configuration,
# because an executable is then in bin/<Config>/ and the object files differ per configuration too.
function(clone26_record_product target kind)
  get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  set(config_dir "")
  if(multi_config)
    set(config_dir "$<CONFIG>/")
  endif()

  if(kind STREQUAL "TEST")
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/products/${config_dir}${target}.executable
         CONTENT "$<TARGET_FILE:${target}>")
  else()
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/products/${config_dir}${target}.objects
         CONTENT "$<TARGET_OBJECTS:${target}>")
  endif()
endfunction()
