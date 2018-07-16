#include <catch2/catch.hpp>
#include "adm/frame.hpp"
#include "adm/elements.hpp"
#include "adm/utilities/id_assignment.hpp"
#include "adm/utilities/object_creation.hpp"

TEST_CASE("adm_frame_test") {
  using namespace adm;

  // create a adm frame
  {
    auto admFrame = Frame::create(FrameStart(std::chrono::nanoseconds(0)),
                                  FrameDuration(std::chrono::nanoseconds(1)),
                                  FrameType("full"));
    auto speechContent = AudioContent::create(AudioContentName("Speech"));

    FrameFormat frameFormat(FrameStart(std::chrono::seconds(10)),
                            FrameDuration(std::chrono::seconds(20)),
                            FrameType("header"));

    auto speechContentFr = AudioContent::create(AudioContentName("Speech"),
                                                AudioContentLanguage("fr"));
    auto speechContentEn = AudioContent::create(AudioContentName("Speech"),
                                                AudioContentLanguage("en"));

    REQUIRE(speechContentEn->has<AudioContentLanguage>());
    REQUIRE(speechContentEn->get<AudioContentLanguage>() == "en");

    auto musicContent = AudioContent::create(AudioContentName("Music"));
    REQUIRE(admFrame->add(speechContent));
    REQUIRE(admFrame->add(musicContent));
    auto narratorObject = AudioObject::create(AudioObjectName("Narrator"));
    REQUIRE(admFrame->add(narratorObject));
    REQUIRE(speechContent->addReference(narratorObject));

    auto stereoPack = AudioPackFormat::create(AudioPackFormatName("stereo"),
                                              TypeDescriptor(0));
    REQUIRE(admFrame->add(stereoPack));

    auto admFrame_2 = Frame::create(FrameStart(std::chrono::nanoseconds(0)),
                                    FrameDuration(std::chrono::nanoseconds(1)),
                                    FrameType("full"));
    REQUIRE_THROWS_AS(admFrame_2->add(speechContent), std::runtime_error);
    REQUIRE_THROWS_AS(admFrame_2->add(musicContent), std::runtime_error);
    REQUIRE_THROWS_AS(admFrame_2->add(narratorObject), std::runtime_error);
    REQUIRE_THROWS_AS(admFrame_2->add(stereoPack), std::runtime_error);
  }
}
