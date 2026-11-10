// Translation unit B of the one-definition-rule guard of design C. It does the same as translation unit A under
// another name. In the shipped target both include the same header and the lists are equal. The mutant twin of
// the target compiles this file against a copy of figure_reflected.hpp with one more class in shapes_reflected, so
// the two lists differ. The twin shows the violation only through the comparison: no file of this guard calls
// clone().
#include <clone26/figure_reflected.hpp>

#include "support/odr_lists.hpp"

namespace clone26::testing_odr {

std::vector<std::string> tu_b_classes() { return searched_class_names<^^shapes_reflected>(); }

}  // namespace clone26::testing_odr
