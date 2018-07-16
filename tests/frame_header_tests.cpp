#include <catch2/catch.hpp>
#include "adm/utilities/id_assignment.hpp"
#include "adm/utilities/object_creation.hpp"
#include "adm/serialized/frame_header.hpp"

TEST_CASE("frame_header") {
  using namespace adm;

  // create a basic document
  {
    FrameHeader frameHeader(FrameStart(std::chrono::nanoseconds(0)),
                            FrameDuration(std::chrono::nanoseconds(1)),
                            FrameType("full"));
  }
}
