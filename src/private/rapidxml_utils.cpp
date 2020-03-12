#include "adm/private/rapidxml_utils.hpp"
#include <algorithm>

namespace adm {
  namespace xml {
    int getDocumentLine(rapidxml::xml_node<>* node) {
      return countLines(node->document()->first_node()->name(), node->name());
    }
    int getDocumentLine(rapidxml::xml_attribute<>* attr) {
      return countLines(attr->document()->first_node()->name(), attr->name());
    }
  }  // namespace xml
}  // namespace adm
