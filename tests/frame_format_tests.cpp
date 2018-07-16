#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#include <catch2/catch.hpp>
#include "adm/elements/time.hpp"
#include "adm/serialized/frame_format.hpp"
#include "adm/serialized/frame_header.hpp"

#include <iostream>

TEST_CASE("frame_format_tests") {
  using namespace adm;
  // Attributes / Elements
  {
    FrameFormat frameFormat(FrameStart(std::chrono::seconds(10)),
                            FrameDuration(std::chrono::seconds(20)),
                            FrameType("header"), TimeReference("total"));

    REQUIRE(frameFormat.has<FrameFormatId>() == true);
    REQUIRE(frameFormat.has<FrameStart>() == true);
    REQUIRE(frameFormat.has<FrameDuration>() == true);
    REQUIRE(frameFormat.has<TimeReference>() == true);
    REQUIRE(frameFormat.has<FrameType>() == true);
    REQUIRE(frameFormat.has<CountToFull>() == false);
    REQUIRE(frameFormat.has<NumSubFrame>() == false);
    REQUIRE(frameFormat.has<FrameSkip>() == false);
    REQUIRE(frameFormat.has<FrameShift>() == false);

    frameFormat.set(CountToFull(1));
    frameFormat.set(NumSubFrame(1));
    frameFormat.set(FrameSkip(1));
    frameFormat.set(FrameShift(1));

    REQUIRE(frameFormat.has<CountToFull>() == true);
    REQUIRE(frameFormat.has<NumSubFrame>() == true);
    REQUIRE(frameFormat.has<FrameSkip>() == true);
    REQUIRE(frameFormat.has<FrameShift>() == true);

    REQUIRE(frameFormat.get<FrameFormatId>().get<FrameFormatIdValue>() == 0u);
    REQUIRE(frameFormat.get<TimeReference>() == "total");
    REQUIRE(frameFormat.get<FrameType>() == "header");
    REQUIRE(frameFormat.get<FrameStart>().get() == std::chrono::seconds(10));
    REQUIRE(frameFormat.get<FrameDuration>().get() == std::chrono::seconds(20));
    REQUIRE(frameFormat.get<CountToFull>().get() == 1u);
    REQUIRE(frameFormat.get<NumSubFrame>().get() == 1u);
    REQUIRE(frameFormat.get<FrameSkip>().get() == 1u);
    REQUIRE(frameFormat.get<FrameShift>().get() == 1u);

    frameFormat.unset<CountToFull>();
    frameFormat.unset<NumSubFrame>();
    frameFormat.unset<FrameSkip>();
    frameFormat.unset<FrameShift>();

    REQUIRE(frameFormat.has<CountToFull>() == false);
    REQUIRE(frameFormat.has<NumSubFrame>() == false);
    REQUIRE(frameFormat.has<FrameSkip>() == false);
    REQUIRE(frameFormat.has<FrameShift>() == false);
  }
}
