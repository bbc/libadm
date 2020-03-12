#include <boost/optional.hpp>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "adm/private/xml_parser_helper.hpp"

namespace adm {
  namespace xml {
    using NodePtr = rapidxml::xml_node<>*;
    /// @brief Implementation details
    namespace detail {

      /**
       * @brief Find the first element within a node with a specified name
       *
       * @returns NodePtr to first element or a nullptr if no element could
       * not be found.
       */
      NodePtr findElement(NodePtr node, const std::string& name) {
        for (NodePtr elementNode = node->first_node(); elementNode;
             elementNode = elementNode->next_sibling()) {
          if (std::string(elementNode->name()) == name) {
            return elementNode;
          }
        }
        return nullptr;
      }

      /**
       * @brief Find all elements within a node with a specified name
       *
       * @returns a vector of NodePtr or an empty vector if no element could
       * not be found.
       */
      std::vector<NodePtr> findElements(NodePtr node, const std::string& name) {
        std::vector<NodePtr> elements;
        for (NodePtr elementNode = node->first_node(); elementNode;
             elementNode = elementNode->next_sibling()) {
          if (std::string(elementNode->name()) == name) {
            elements.push_back(elementNode);
          }
        }
        return elements;
      }
    }  // namespace detail
    

    FormatDescriptor checkFormat(
        boost::optional<FormatDescriptor> formatLabel,
        boost::optional<FormatDescriptor> formatDefinition) {
      if (formatLabel != boost::none && formatDefinition != boost::none) {
        if (formatLabel.get() != formatDefinition.get()) {
          throw std::runtime_error(
              "formatLabel and formatDefinition do not match");
        }
        return formatLabel.get();
      }
      if (formatLabel != boost::none) {
        return formatLabel.get();
      }
      if (formatDefinition != boost::none) {
        return formatDefinition.get();
      }
      throw std::runtime_error(
          "Neither FormatLabel nor formatDefinition found");
    }

  }  // namespace xml
}  // namespace adm
