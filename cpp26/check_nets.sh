#!/usr/bin/env bash
# The adversarial check of the four nets and of the one-definition-rule guard of design C.
#
# The shipped hierarchy has one class that forgot its overrides, ForgetfulTile, and one that did not, Tile. Each
# net is written to see ForgetfulTile and to accept the rest, so in the shipped build every net has to pass.
# Each net also has a mutant twin: the same sources compiled against a copy of figure_clone.hpp in which Tile lost
# its overrides too. The copy lives in the build directory and is made by cmake/Clone26Mutant.cmake, which deletes
# the two lines of Tile's hooks. Every twin has to fail, and for its own reason: the failure has to show the
# signature of that net.
#
# There are four nets and five targets: net 1, the contract net, has an observe target that counts violations
# (net_contract) and an enforce target that dies (net_contract_death).
#
# A sixth target, design_c_odr, is not a net. It guards the one definition rule of design C: two translation units
# of one executable must see the same classes in the searched namespace. Its twin compiles one of the two against a
# copy of figure_reflected.hpp with one more class, so it has to fail the comparison of the lists of classes.
#
# A net that does not exist (CMake skipped it) counts as a failure, because a net that is not there catches nothing.
#
# Exit status 0 only if every target passes and every mutant twin fails, each with its own signature.
#
# Environment:
#   CXX                  the compiler (default: g++-16 if it is on PATH, else g++)
#   CXXFLAGS             the compiler flags, replacing the default (-fcontracts -freflection, which GCC 16 needs)
#   CLONE26_NETS_DIR     the build directory (default: build-nets next to this script)
#   CLONE26_NETS_CONFIG  the configuration to build and run with a multi-config generator such as Ninja Multi-Config
#                        (default: Release). A single-config generator ignores it.
#   CMAKE_GENERATOR      honoured as CMake honours it. Without it the script uses Ninja when it finds one.
set -u -o pipefail

here=$(cd "$(dirname "$0")" && pwd)
cxx=${CXX:-$(command -v g++-16 || command -v g++)}
flags=${CXXFLAGS--fcontracts -freflection}
build_dir=${CLONE26_NETS_DIR:-$here/build-nets}

nets="net_contract net_contract_death net_constexpr net_lint net_generated design_c_odr"
failures=0
evidence_width=150  # characters of the failure line that the report shows

# What the failure of the mutant twin of each net has to contain: one extended regular expression per line.
signature() {
  case $1 in
    net_contract) printf '%s\n' 'FAILED  \] NetContractCountTest\.Tile_' 'violations' ;;
    net_contract_death) printf '%s\n' 'FAILED  \] NetContractDeathTest\.Tile_' 'contract violation' ;;
    net_constexpr) printf '%s\n' 'static assertion failed' 'keeps_type<shapes::Tile>' ;;
    net_lint) printf '%s\n' 'static assertion failed' 'it names: Tile, ForgetfulTile' ;;
    net_generated) printf '%s\n' 'FAILED  \]' ': Tile$' ;;
    design_c_odr) printf '%s\n' 'FAILED  \] DesignCOdr\.every_translation_unit' '"Extra"' ;;
  esac
}

configure() {
  local generator=()
  if [ -z "${CMAKE_GENERATOR:-}" ] && command -v ninja >/dev/null 2>&1; then generator=(-G Ninja); fi
  cmake -S "$here" -B "$build_dir" ${generator[@]+"${generator[@]}"} -DCMAKE_CXX_COMPILER="$cxx" \
    -DCMAKE_CXX_FLAGS="$flags" >"$build_dir.configure.log" 2>&1
}

# CMake writes what each target produces into <build dir>/products/ (see cmake/Clone26Targets.cmake), so the script
# does not know where a generator puts an executable. A multi-config generator has one directory per configuration.
detect_configuration() {
  if grep -q '^CMAKE_CONFIGURATION_TYPES:' "$build_dir/CMakeCache.txt"; then
    build_args=(--config "${CLONE26_NETS_CONFIG:-Release}")
    products_dir="$build_dir/products/${CLONE26_NETS_CONFIG:-Release}"
  else
    build_args=()
    products_dir="$build_dir/products"
  fi
}

has_target() { # <target>
  [ -f "$products_dir/$1.executable" ] || [ -f "$products_dir/$1.objects" ]
}

is_executable() { # <target>
  [ -f "$products_dir/$1.executable" ]
}

# Builds a target and, when it is an executable, runs it. The output goes to <build dir>/<target>.log.
# Returns 0 if both worked.
run_target() { # <target>
  local log="$build_dir/$1.log" executable
  cmake --build "$build_dir" ${build_args[@]+"${build_args[@]}"} --target "$1" >"$log" 2>&1 || return 1
  if is_executable "$1"; then
    executable=$(<"$products_dir/$1.executable")
    "$executable" >>"$log" 2>&1 || return 1
  fi
  return 0
}

# Prints the first line of the log that shows the first pattern of the signature, and fails when any pattern of
# the signature is missing from the log.
evidence() { # <log> <net>
  local pattern first=1 line=""
  while IFS= read -r pattern; do
    if ! grep -Eq -- "$pattern" "$1"; then return 1; fi
    if [ "$first" = 1 ]; then line=$(grep -E -m1 -- "$pattern" "$1"); first=0; fi
  done < <(signature "$2")
  printf '%s' "$line" | cut -c1-"$evidence_width"
}

mkdir -p "$(dirname "$build_dir")"
echo "compiler: $cxx $flags"
echo
configure || { echo "configure failed, see $build_dir.configure.log"; exit 1; }
detect_configuration

echo "== the nets and the guard: each has to pass, which means a net sees ForgetfulTile and accepts the other classes"
for net in $nets; do
  if ! has_target "$net"; then
    echo "  $net: MISSING, CMake did not create it (see the skipping lines in $build_dir.configure.log)"
    failures=$((failures + 1))
  elif run_target "$net"; then
    echo "  $net: PASSED"
  else
    echo "  $net: FAILED, see $build_dir/$net.log"
    failures=$((failures + 1))
  fi
done
echo

echo "== the mutant twins (Tile loses its overrides, or a class is added): each has to fail, for its own reason"
for net in $nets; do
  if ! has_target "${net}_mutant"; then
    echo "  ${net}_mutant: MISSING, CMake did not create it"
    failures=$((failures + 1))
  elif run_target "${net}_mutant"; then
    echo "  ${net}_mutant: PASSED, but it has to fail when Tile loses its overrides"
    failures=$((failures + 1))
  elif shown=$(evidence "$build_dir/${net}_mutant.log" "$net"); then
    echo "  ${net}_mutant: FAILED as expected: $shown"
  else
    echo "  ${net}_mutant: FAILED, but without its signature, see $build_dir/${net}_mutant.log"
    failures=$((failures + 1))
  fi
done
echo

if [ "$failures" -eq 0 ]; then
  echo "check_nets: OK, all four nets and the design C guard pass and all their mutant twins fail"
  exit 0
fi
echo "check_nets: $failures problem(s)"
exit 1
