#include <catch2/catch.hpp>
#include "adm/document.hpp"
#include "adm/elements.hpp"
#include "adm/frame.hpp"
#include "adm/utilities/id_assignment.hpp"
#include "adm/utilities/object_creation.hpp"
#include "adm/write.hpp"
#include "helper/file_comparator.hpp"

std::shared_ptr<const adm::Document> createSimpleScene();
std::shared_ptr<const adm::Frame> createSimpleFrame();

TEST_CASE("simple_scene_default") {
  using namespace adm;
  auto document = createSimpleScene();
  // with ebu core wrapper structure
  std::stringstream xml;
  writeXml(xml, document);
  CHECK_THAT(xml.str(), EqualsXmlFile("simple_scene_default"));
}

TEST_CASE("simple_scene_itu") {
  using namespace adm;
  auto document = createSimpleScene();

  std::stringstream xml;
  writeXml(xml, document,
           xml::WriterOptions::write_default_values |
               xml::WriterOptions::itu_structure);

  CHECK_THAT(xml.str(), EqualsXmlFile("simple_scene_itu"));
}

TEST_CASE("write_optional_defaults") {
  using namespace adm;
  auto document = createSimpleScene();

  std::stringstream xml;
  writeXml(xml, document, xml::WriterOptions::write_default_values);

  CHECK_THAT(xml.str(), EqualsXmlFile("write_optional_defaults"));
}

TEST_CASE("write_frame") {
  using namespace adm;
  auto frame = createSimpleFrame();

  std::stringstream xml;
  writeXml(xml, frame);

  CHECK_THAT(xml.str(), EqualsXmlFile("xml_writer_simple_frame"));
}

TEST_CASE("write_complementary_audio_objects") {
  using namespace adm;

  auto audioObjectDefault = AudioObject::create(AudioObjectName("Default"));
  auto audioObjectComplementary =
      AudioObject::create(AudioObjectName("Complementary"));

  audioObjectDefault->addComplementary(audioObjectComplementary);

  auto document = Document::create();
  document->add(audioObjectDefault);

  std::stringstream xml;
  writeXml(xml, document);

  CHECK_THAT(xml.str(), EqualsXmlFile("write_complementary_audio_objects"));
}

std::shared_ptr<const adm::Document> createSimpleScene() {
  using namespace adm;
  auto document = Document::create();
  auto start = Start(parseTimecode("10:00:00.0"));
  auto end = End(parseTimecode("10:00:10.0"));
  auto programme =
      AudioProgramme::create(AudioProgrammeName("Main"), start, end);
  auto content = AudioContent::create(AudioContentName("Main"));
  programme->addReference(content);

  auto result = createSimpleObject("MainObject");
  content->addReference(result.audioObject);

  auto channel = result.audioChannelFormat;
  channel->add(AudioBlockFormatObjects(SphericalPosition(Azimuth(30)),
                                       Rtime(std::chrono::seconds(0)),
                                       JumpPosition(JumpPositionFlag(true))));
  channel->add(AudioBlockFormatObjects(
      SphericalPosition(Azimuth(-30)), Rtime(std::chrono::seconds(3)),
      JumpPosition(JumpPositionFlag(true),
                   InterpolationLength(std::chrono::seconds(1)))));
  channel->add(AudioBlockFormatObjects(SphericalPosition(Azimuth(0)),
                                       Rtime(std::chrono::seconds(6))));
  channel->add(AudioBlockFormatObjects(SphericalPosition(Azimuth(30)),
                                       Rtime(std::chrono::seconds(9))));

  document->add(programme);
  reassignIds(document);

  return document;
}

std::shared_ptr<const adm::Frame> createSimpleFrame() {
  using namespace adm;

  auto admFrame = Frame::create(FrameStart(std::chrono::nanoseconds(0)),
                                FrameDuration(std::chrono::nanoseconds(1)),
                                FrameType("full"));
  auto start = Start(parseTimecode("00:00:00.0"));
  auto end = End(parseTimecode("00:00:10.0"));
  auto programme =
      AudioProgramme::create(AudioProgrammeName("Main"), start, end);
  auto content = AudioContent::create(AudioContentName("Main"));
  programme->addReference(content);

  auto result = createSimpleObject("MainObject");
  content->addReference(result.audioObject);

  auto channel = result.audioChannelFormat;
  channel->add(AudioBlockFormatObjects(SphericalPosition(Azimuth(30)),
                                       Rtime(std::chrono::seconds(0)),
                                       JumpPosition(JumpPositionFlag(true))));
  channel->add(AudioBlockFormatObjects(
      SphericalPosition(Azimuth(-30)), Rtime(std::chrono::seconds(3)),
      JumpPosition(JumpPositionFlag(true),
                   InterpolationLength(std::chrono::seconds(1)))));
  channel->add(AudioBlockFormatObjects(SphericalPosition(Azimuth(0)),
                                       Rtime(std::chrono::seconds(6))));
  channel->add(AudioBlockFormatObjects(SphericalPosition(Azimuth(30)),
                                       Rtime(std::chrono::seconds(9))));
  admFrame->add(programme);
  reassignIds(admFrame);

  auto trackFormat = TransportTrackFormat();
  AudioTrack audioTrack(TrackId(1));
  audioTrack.add(result.audioTrackUid->get<AudioTrackUidId>());
  trackFormat.add(audioTrack);
  admFrame->frameHeader().add(trackFormat);

  return admFrame;
}
