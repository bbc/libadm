#pragma once

#include "rapidxml/rapidxml.hpp"
#include <algorithm>

namespace adm {
  namespace xml {
    template <typename It>
    int countLines(It begin, It end) {
      return std::count(begin, end, '\n');
    }

    int getDocumentLine(rapidxml::xml_node<>* node);

    int getDocumentLine(rapidxml::xml_attribute<>* attr);
  }  // namespace xml
}  // namespace adm
