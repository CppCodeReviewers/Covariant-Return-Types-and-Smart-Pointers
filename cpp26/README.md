# cpp26: the clone() designs of this article in C++26

This directory holds a C++26 edition of the `clone()` designs of the 2014 article. It is a separate CMake project with
GoogleTest suites. The 2014 text and code in the repository root are unchanged. The `2026 edition` section of the
root README says how this project relates to the article.

## Designs

| design | header | namespace | `clone()` | needs |
|---|---|---|---|---|
| A, hooks | `include/clone26/cloneable.hpp`, `figure_clone.hpp` | `clone26`, `shapes` | `Cloneable::clone(this Self&&)` returns `unique_ptr<Derived>` and calls two private virtual hooks | explicit object parameters |
| B, `std::polymorphic` | `figure_polymorphic.hpp` | `shapes_polymorphic` | none: the wrapper copies the dynamic type | `std::polymorphic` in the library |
| C, reflection mixin | `auto_cloneable.hpp`, `figure_reflected.hpp` | `clone26`, `shapes_reflected` | `AutoCloneable::clone(this Self&&)` reflects over the namespace of the type and copies with the class that matches the dynamic type | reflection |

Design A and design C are mixins with no template parameters, so any class can use them, not only a `Figure`. In
design A the copy and move operations of the bases are protected and defaulted. That blocks slicing by assignment
through a base reference and keeps every derived class assignable. A deleted assignment in a base, even with a reason
(`= delete("...")`), would delete the derived one too, and `tests/delete_reason_test.cpp` states that as type traits.

**Design C has a requirement: every translation unit of one program must see the same, finished set of classes in the
searched namespace.** `clone()` is a template, so the dispatch over that set is generated in each translation unit that
calls it, and the linker keeps one copy of each specialization. Two translation units that see different classes in the
namespace make the same specialization with different bodies. That violates the one definition rule, no diagnostic is
required, and the symptom can be a `clone_unavailable` for a class that the caller can see. So:

- Define the whole hierarchy in one header. `include/clone26/figure_reflected.hpp` is that header here.
- Add nothing to the namespace in another translation unit, and nothing after the first `clone()` instantiation.
- To extend, put a class in another namespace. Its own namespace is searched for it, so it is cloned through its own
  type (`tests/design_c_test.cpp` shows it).

The requirement is checkable. `clone26::searched_class_names<Namespace>()` lists the classes that `clone()` searches, as
the translation unit sees them. It has internal linkage and is a template, so every translation unit instantiates its own
copy. With external linkage the linker merges the copies and the difference between two units is invisible, which was
measured: two units with 3 and 4 classes compared equal. `design_c_odr` links two translation units that export their
lists under two names (`design_c_tu_a.cpp` and `design_c_tu_b.cpp`), and a third compares them. A program that uses design
C should have such a test.

Design C has a limit, and a test shows it: the classes it can copy are the ones in the namespace of the type that
`clone()` is called on. A class derived from `Figure` in another namespace, cloned through a base reference, makes
`clone()` throw `clone26::clone_unavailable`, a `std::logic_error` whose message holds `typeid(source).name()`. It does
not return an empty pointer that a caller could dereference by mistake. The failure shows at run time, so a test or a
lint should cover every class that uses the mixin. A class cloned through its own type is in the set and does not
throw.

Only complete classes count, and each class counts once. A forward declaration (`struct Pending;`) is ignored, because
an incomplete class has no bases to ask about. An alias (`using TileAlias = Tile;`) is skipped, because it names a class
a second time, and a search that took it for another class moved from an rvalue twice. The nets use the same filter,
and the namespaces of the figures have a `Pending` and a `TileAlias` so that the tests exercise it.

## The nets

A class that derives from a `Cloneable` and forgets to override `copy_` and `move_` is cloned as its nearest ancestor
that does, and the downcast in `clone()` is undefined behaviour that no compiler warns about. `ForgetfulTile` is such
a class, and the correct `Tile` sits next to it in `figure_clone.hpp`. Four nets catch it. Each is written to see
`ForgetfulTile` and to accept the other classes, so in the shipped build every net passes.

| net | target | how it sees the forgotten override |
|---|---|---|
| 1, contracts | `net_contract` | observe semantic and a replacement `handle_contract_violation` that counts: none for the correct classes, one per clone of `ForgetfulTile` |
| 1, contracts | `net_contract_death` | enforce semantic: `EXPECT_DEATH` on `ForgetfulTile`, a clean exit for `Tile` |
| 2, constant evaluation | `net_constexpr` | `static_assert` on a round trip and on whether a clone through the derived type is a constant expression |
| 3a, reflection lint | `net_lint` | the classes without both overrides are exactly `{ForgetfulTile}` |
| 3b, generated tests | `net_generated` | one test body, `template for` over every concrete `Figure`, expects `false` for `ForgetfulTile` only |

`check_nets.sh` also proves that the nets can fail. CMake copies `figure_clone.hpp` into the build directory with the
two hook lines of `Tile` deleted, and every net has a `<net>_mutant` twin that compiles the same sources against that
copy. The script builds and runs the nets, which have to pass, then the twins, which have to fail, each with the
signature of its own net. A net that CMake skipped counts as a failure. It exits 0 only if every net passes and every
twin fails.

The script treats the guard of design C the same way, although it is not a net. Its twin, `design_c_odr_mutant`, compiles
`design_c_tu_b.cpp` against a second mutant, a copy of `figure_reflected.hpp` with one more class in `shapes_reflected`,
and links it with the unchanged translation unit A. The two units then see different classes and the comparison fails
and names `Extra`. The twin shows the violation only through that comparison: no file of the guard calls `clone()`, so
it never runs the dispatch that the violation would break. Like the other twins it is left out of the default build and
of CTest.

The script works with any generator. It does not look for executables in `bin/`: a multi-config generator such as
`Ninja Multi-Config` puts them in `bin/<Config>/`. CMake writes what each target produces into `<build dir>/products/`
(the path of an executable, or the object files of a build-only target, in a directory per configuration when the
generator has several), and the script reads those files. It detects a multi-config generator from
`CMAKE_CONFIGURATION_TYPES` in the cache and builds with `--config Release`, or with the configuration in
`CLONE26_NETS_CONFIG`. `CMAKE_GENERATOR='Ninja Multi-Config' cpp26/check_nets.sh` gives the same results as the default
Ninja run.

## No preprocessor conditionals, on purpose

No header and no test has an `#if`, `#ifdef`, `#ifndef` or `#define`. This is deliberate, and it is the practice the
project recommends wherever C++26 has a language answer:

- A feature-test macro can say yes where the feature does not work. GCC 16.2 defines `__cpp_contracts` for plain
  `-std=c++26`, and a `contract_assert` then compiles but fails to link without `-fcontracts`.
- A conditional makes the compiler see different code in each configuration, and only the configuration you build is
  ever checked.

So each feature is a probe in `cmake/Clone26Features.cmake` that compiles and links a small program with the standard
and the flags of the project. A target that needs a missing feature is not created, CMake prints a `skipping` line,
and a header is only included by targets whose probe passed. A variant is a separate header or target.

The contracts variant works by include path. `include/clone26/clone_check.hpp` does nothing. The file of the same name
in `contracts/include/clone26/` has a `contract_assert`. A target that links `clone26_contracts` finds the second one
first, and `Cloneable::clone()` calls `clone26::check_clone_type` in both cases.

## One flavour per executable, one set of classes per program

Both `clone_check.hpp` files define `clone26::check_clone_type`. Two translation units of one program that see
different ones violate the one definition rule, and no diagnostic is required. Every executable of this project is
built from one flavour, and each design is its own executable. The designs use different namespaces, so they could be
combined, and a project that does so should still keep one flavour of `clone_check.hpp` per program.

The same rule has a second form in design C, described with its requirement above: every translation unit of a program
must see the same set of classes in the searched namespace.

## Where the project differs from the posts

The examples of the posts (`posts/115` to `posts/118` of the examples repository) are single files. The project
differs from them in these ways, and the posts are not changed:

- The contract violation message names `clone26::check_clone_type`, not `Cloneable::clone(this Self&&)`, because
  `clone()` calls that function and the `contract_assert` is in it.
- `Cloneable` and `AutoCloneable` live in the namespace `clone26`, and `shapes` has `using clone26::Cloneable;`.
- Design C is in `shapes_reflected`, so that the three designs have distinct names.
- The contract net is two targets, observe and enforce, where the post shows one program and names the semantics.
- The lint requires exactly `{ForgetfulTile}` in `net_lint`, as the post does, and the mutant twins add the check that
  a `Tile` that lost its overrides makes every net fail.
- `Tile` has a `Tile(double, int)` constructor and a defaulted `Tile()`, so tests build it in one expression.
- A moved-from donor is checked through the vector buffer, which a move construction hands over (a guarantee of
  [container.reqmts]), not through `history.empty()`, which the standard does not promise.

## Build

```sh
cmake -S cpp26 -B build -DCMAKE_CXX_COMPILER=g++-16 -DCMAKE_CXX_FLAGS="-fcontracts -freflection"
cmake --build build
ctest --test-dir build
cpp26/check_nets.sh
```

GCC 16 builds every target. Clang 22 with libc++ builds the design A tests, the delete-reason test and the constexpr
net, and CMake prints the `skipping` lines for the rest. CMake 3.30 is the minimum, because CMake 3.29 cannot build
C++26 (it stops with `requires the language dialect "CXX26"`). GoogleTest is fetched at a pinned commit.

## Links

The episodes: [1](https://wrocpp.github.io/posts/clone-helper-ub-forgotten-override/),
[2](https://wrocpp.github.io/posts/deducing-this-clone-without-covariance/),
[3](https://wrocpp.github.io/posts/catch-a-forgotten-clone-override/),
[4](https://wrocpp.github.io/posts/std-polymorphic-instead-of-clone/).
The examples they run on Compiler Explorer:
[115](https://github.com/wrocpp/cpp26-reflection-examples/tree/main/posts/115-clone-2014-forgotten-override),
[116](https://github.com/wrocpp/cpp26-reflection-examples/tree/main/posts/116-deducing-this-clone),
[117](https://github.com/wrocpp/cpp26-reflection-examples/tree/main/posts/117-forgotten-override-nets),
[118](https://github.com/wrocpp/cpp26-reflection-examples/tree/main/posts/118-polymorphic-instead-of-clone).
