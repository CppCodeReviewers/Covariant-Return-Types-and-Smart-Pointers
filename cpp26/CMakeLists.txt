cmake_minimum_required(VERSION 3.30)

# Set before project(): -freflection is only accepted together with -std=c++26, and the compiler check that
# project() runs uses the flags it is configured with.
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(clone26 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Clone26Features)

# The headers are a library of their own. Every target links it, so every target gets the include path and the
# warnings.
add_library(clone26 INTERFACE)
target_include_directories(clone26 INTERFACE include)
target_compile_features(clone26 INTERFACE cxx_std_26)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(clone26 INTERFACE -Wall -Wextra -Wpedantic)
endif()

# The contracts flavour of clone_check.hpp has the same include name as the default one. A target that links this
# one finds it first. The target exists only when the feature check found contracts that compile and link.
if(CLONE26_HAVE_CONTRACTS)
  add_library(clone26_contracts INTERFACE)
  target_include_directories(clone26_contracts BEFORE INTERFACE contracts/include)
  target_link_libraries(clone26_contracts INTERFACE clone26)
endif()

include(Clone26Mutant)

include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG 063de7e9578f82b369302001269680b4b1553359  # v1.18.0
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
