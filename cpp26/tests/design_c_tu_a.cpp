// Translation unit A of the one-definition-rule guard of design C. It includes the hierarchy and exports the list of
// classes that clone() searches, as this translation unit sees it, under a name of its own.
#include <clone26/figure_reflected.hpp>

#include "support/odr_lists.hpp"

namespace clone26::testing_odr {

std::vector<std::string> tu_a_classes() { return searched_class_names<^^shapes_reflected>(); }

}  // namespace clone26::testing_odr
