/// @file parse_opt.hpp
#pragma once
#include <string>
#include <memory>
#include <iosfwd>
#include "adm/detail/enum_bitmask.hpp"
#include "adm/export.h"

namespace adm {

  class Document;

  namespace xml {
    /**
     * @brief Representation of available options to influence the behaviour
     * of the XML parser.
     *
     * `ParserOptions` satisfies the requirements of
     * [BitmaskType](http://en.cppreference.com/w/cpp/concept/BitmaskType).
     *
     * This means that the bitwise operators (e.g. `operator|` or `operator&`)
     * are defined for this type. Thus options may be combined by `OR`-ing the
     * respective values.
     *
     * @note No options have been implemented so far. As soon as this is done,
     * provide an usage example, a list describing the member constants
     * (options) and which options can be combined. Refer to
     * [std::filesystem::copy_options](http://en.cppreference.com/w/cpp/filesystem/copy_options)
     * for an example.
     * @ingroup xml
     */
    enum class ParserOptions : unsigned {
      none = 0x0,  ///< default behaviour
      recursive_node_search =
          0x1,  ///< recursively search whole xml for audioFormatExtended node
    };
  }  // namespace xml
}  // namespace adm

ENABLE_ENUM_BITMASK_OPERATORS(adm::xml::ParserOptions);
