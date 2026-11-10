# The mutant headers. A mutant is a copy of a header of the project, written into the build directory at configure
# time with one change that the shipped code must never have. A target that has a "mutant" twin gets a second
# target with the same sources that finds the mutant first on its include path. check_nets.sh builds the twins and
# expects every one to fail. That shows each check notices the change, without a macro in a header and without a
# second copy of the check.
#
# There are two mutants.
#
# The Tile mutant: figure_clone.hpp without the two hook lines of Tile (the lines that name make_unique<Tile>), so
# that Tile forgets its overrides. The four nets use it.
#
# The extra-class mutant: figure_reflected.hpp with one more concrete class in shapes_reflected, inserted after the
# alias line. The guard for the one definition rule of design C uses it for one translation unit of its twin, so the
# two translation units of the executable see different sets of classes.
#
# Each mutation checks how often its marker occurs in the header and stops the configure when the count is not the
# expected one, so a mutation that no longer applies is an error and not a twin that quietly mutates nothing.

set(CLONE26_MUTANT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/mutant/include)
set(CLONE26_MUTANT_EXTRA_CLASS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/mutant-extra-class/include)

function(clone26_count_occurrences text needle result)
  string(LENGTH "${text}" length_before)
  string(REPLACE "${needle}" "" stripped "${text}")
  string(LENGTH "${stripped}" length_after)
  string(LENGTH "${needle}" needle_length)
  math(EXPR count "(${length_before} - ${length_after}) / ${needle_length}")
  set(${result} ${count} PARENT_SCOPE)
endfunction()

function(clone26_read_for_mutation header marker expected result)
  set(source ${CMAKE_CURRENT_SOURCE_DIR}/include/clone26/${header})
  file(READ ${source} text)
  clone26_count_occurrences("${text}" "${marker}" found)
  if(NOT found EQUAL expected)
    message(FATAL_ERROR "The mutation of ${header} needs ${expected} occurrence(s) of '${marker}' in ${source}, "
                        "found ${found}. Update cmake/Clone26Mutant.cmake with the header.")
  endif()
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${source})
  set(${result} "${text}" PARENT_SCOPE)
endfunction()

function(clone26_write_tile_mutant)
  set(hook_marker "make_unique<Tile>")
  set(hooks_in_tile 2)

  clone26_read_for_mutation(figure_clone.hpp "${hook_marker}" ${hooks_in_tile} text)
  string(REGEX REPLACE "[^\n]*${hook_marker}[^\n]*\n" "" mutated "${text}")
  clone26_count_occurrences("${mutated}" "${hook_marker}" left)
  if(NOT left EQUAL 0)
    message(FATAL_ERROR "The Tile mutation left ${left} line(s) with ${hook_marker} in the mutant header.")
  endif()

  file(WRITE ${CLONE26_MUTANT_INCLUDE_DIR}/clone26/figure_clone.hpp "${mutated}")
endfunction()

function(clone26_write_extra_class_mutant)
  set(anchor "using TileAlias = Tile;")
  set(extra_class "struct Extra : Square {};")

  clone26_read_for_mutation(figure_reflected.hpp "${anchor}" 1 text)
  string(REPLACE "${anchor}" "${anchor}\n${extra_class}" mutated "${text}")
  clone26_count_occurrences("${mutated}" "${extra_class}" added)
  if(NOT added EQUAL 1)
    message(FATAL_ERROR "The extra-class mutation added ${added} class(es) to the mutant header, not 1.")
  endif()

  file(WRITE ${CLONE26_MUTANT_EXTRA_CLASS_INCLUDE_DIR}/clone26/figure_reflected.hpp "${mutated}")
endfunction()

clone26_write_tile_mutant()
clone26_write_extra_class_mutant()
