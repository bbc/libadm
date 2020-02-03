#include <catch2/catch.hpp>
#include "adm/combiner.hpp"
#include "adm/document.hpp"
#include "adm/elements.hpp"
#include "adm/frame.hpp"
#include "adm/segmenter.hpp"
#include "adm/utilities/copy.hpp"
#include "adm/utilities/id_assignment.hpp"
#include "adm/utilities/object_creation.hpp"

#include <iostream>

TEST_CASE("simple_combiner") {
  using namespace adm;

  // create a basic document
  auto document = Document::create();
  auto programme = AudioProgramme::create(AudioProgrammeName("Programme"),
                                          Start(std::chrono::seconds(10)));
  auto content = AudioContent::create(AudioContentName("Content"));
  auto simpleObject = createSimpleObject("Object");
  simpleObject.audioObject->set(Start(std::chrono::seconds(10)));
  programme->addReference(content);
  content->addReference(simpleObject.audioObject);
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(500))));
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(1250))));
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(2000))));
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(2500))));
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(3000))));

  document->add(programme);
  reassignIds(document);

  Segmenter segmenter(document);
  Combiner combiner;
  for (unsigned int i = 0; i < 30; ++i) {
    auto frame = segmenter.getFrame(SegmentStart(std::chrono::seconds(i)),
                                    SegmentDuration(std::chrono::seconds(1)));
    combiner.push(frame);
  }
  auto combinedDocument = combiner.getDocument();
  // AudioProgramme
  REQUIRE(combinedDocument->getElements<AudioProgramme>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioProgramme>()[0] !=
          document->getElements<AudioProgramme>()[0]);
  REQUIRE(combinedDocument->getElements<AudioProgramme>()[0]
              ->get<AudioProgrammeName>() == "Programme");
  REQUIRE(combinedDocument->getElements<AudioProgramme>()[0]
              ->getReferences<AudioContent>()
              .size() == 1);
  // AudioContent
  REQUIRE(combinedDocument->getElements<AudioContent>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioContent>()[0] !=
          document->getElements<AudioContent>()[0]);
  REQUIRE(combinedDocument->getElements<AudioContent>()[0]
              ->get<AudioContentName>() == "Content");
  REQUIRE(combinedDocument->getElements<AudioContent>()[0]
              ->getReferences<AudioObject>()
              .size() == 1);
  // AudioObject
  REQUIRE(combinedDocument->getElements<AudioObject>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioObject>()[0] !=
          document->getElements<AudioObject>()[0]);
  REQUIRE(
      combinedDocument->getElements<AudioObject>()[0]->get<AudioObjectName>() ==
      "Object");
  REQUIRE(combinedDocument->getElements<AudioObject>()[0]
              ->getReferences<AudioPackFormat>()
              .size() == 1);
  REQUIRE(combinedDocument->getElements<AudioObject>()[0]
              ->getReferences<AudioTrackUid>()
              .size() == 1);
  // AudioPackFormat
  REQUIRE(combinedDocument->getElements<AudioPackFormat>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioPackFormat>()[0] !=
          document->getElements<AudioPackFormat>()[0]);
  REQUIRE(combinedDocument->getElements<AudioPackFormat>()[0]
              ->get<AudioPackFormatName>() == "Object");
  REQUIRE(combinedDocument->getElements<AudioPackFormat>()[0]
              ->getReferences<AudioChannelFormat>()
              .size() == 1);
  // AudioChannelFormat
  REQUIRE(combinedDocument->getElements<AudioChannelFormat>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioChannelFormat>()[0] !=
          document->getElements<AudioChannelFormat>()[0]);
  REQUIRE(combinedDocument->getElements<AudioChannelFormat>()[0]
              ->get<AudioChannelFormatName>() == "Object");
  REQUIRE(combinedDocument->getElements<AudioChannelFormat>()[0]
              ->getElements<AudioBlockFormatObjects>()
              .size() == 5);
  // AudioTrackFormat
  REQUIRE(combinedDocument->getElements<AudioTrackFormat>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioTrackFormat>()[0] !=
          document->getElements<AudioTrackFormat>()[0]);
  REQUIRE(combinedDocument->getElements<AudioTrackFormat>()[0]
              ->get<AudioTrackFormatName>() == "Object");
  REQUIRE(combinedDocument->getElements<AudioTrackFormat>()[0]
              ->getReference<AudioStreamFormat>() != nullptr);
  // AudioStreamFormat
  REQUIRE(combinedDocument->getElements<AudioStreamFormat>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioStreamFormat>()[0] !=
          document->getElements<AudioStreamFormat>()[0]);
  REQUIRE(combinedDocument->getElements<AudioStreamFormat>()[0]
              ->getAudioTrackFormatReferences()
              .size() == 1);
  // AudioTrackUid
  REQUIRE(combinedDocument->getElements<AudioTrackUid>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioTrackUid>()[0] !=
          document->getElements<AudioTrackUid>()[0]);
  REQUIRE(combinedDocument->getElements<AudioTrackUid>()[0]
              ->getReference<AudioTrackFormat>() != nullptr);
}

TEST_CASE("combine_transport_trackformats") {
  using namespace adm;
  auto frame =
      Frame::create(FrameStart(std::chrono::seconds(0)),
                    FrameDuration(std::chrono::seconds(1)), FrameType("full"));
  TransportTrackFormat transportTrackFormat(parseTransportId("TP_0001"),
                                            TransportName("file"));
  AudioTrack audioTrack_0(TrackId{0});
  audioTrack_0.add(parseAudioTrackUidId("ATU_00000001"));
  audioTrack_0.add(parseAudioTrackUidId("ATU_00000002"));
  AudioTrack audioTrack_1(TrackId{1});
  audioTrack_1.add(parseAudioTrackUidId("ATU_00000003"));
  AudioTrack audioTrack_2(TrackId{2});
  audioTrack_2.add(parseAudioTrackUidId("ATU_00000004"));
  transportTrackFormat.add(audioTrack_0);
  transportTrackFormat.add(audioTrack_1);
  transportTrackFormat.add(audioTrack_2);
  frame->frameHeader().add(transportTrackFormat);

  Combiner combiner;
  combiner.push(frame);
  combiner.push(frame);

  auto transportTrackFormatCombiner = combiner.getTransportTrackFormat();
  REQUIRE(transportTrackFormat.get<NumTracks>().get() == 3u);
  REQUIRE(transportTrackFormatCombiner.get<NumTracks>().get() == 3u);
  REQUIRE(transportTrackFormat.get<NumIds>().get() == 4u);
  REQUIRE(transportTrackFormatCombiner.get<NumIds>().get() == 4u);
}

TEST_CASE("combine_with_rtime_0") {
  using namespace adm;

  // create a basic document
  auto document = Document::create();
  auto programme = AudioProgramme::create(AudioProgrammeName("Programme"));
  auto content = AudioContent::create(AudioContentName("Content"));
  auto simpleObject = createSimpleObject("Object");
  programme->addReference(content);
  content->addReference(simpleObject.audioObject);
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(0)),
      Duration(std::chrono::milliseconds(1250))));
  simpleObject.audioChannelFormat->add(AudioBlockFormatObjects(
      SphericalPosition(), Rtime(std::chrono::milliseconds(1250))));

  document->add(programme);

  Segmenter segmenter(document);
  Combiner combiner;
  for (unsigned int i = 0; i < 30; ++i) {
    auto frame = segmenter.getFrame(SegmentStart(std::chrono::seconds(i)),
                                    SegmentDuration(std::chrono::seconds(1)));
    combiner.push(frame);
  }
  auto combinedDocument = combiner.getDocument();
  REQUIRE(combinedDocument->getElements<AudioChannelFormat>().size() == 1);
  REQUIRE(combinedDocument->getElements<AudioChannelFormat>()[0]
              ->getElements<AudioBlockFormatObjects>()
              .size() == 2);
}

TEST_CASE("combine_transport_trackformats_only_one") {
  using namespace adm;
  auto frame =
      Frame::create(FrameStart(std::chrono::seconds(0)),
                    FrameDuration(std::chrono::seconds(1)), FrameType("full"));
  TransportTrackFormat transportTrackFormat_1(parseTransportId("TP_0001"),
                                              TransportName("file"));
  TransportTrackFormat transportTrackFormat_2(parseTransportId("TP_0002"),
                                              TransportName("file"));
  AudioTrack audioTrack_1(TrackId{0});
  audioTrack_1.add(parseAudioTrackUidId("ATU_00000001"));
  transportTrackFormat_1.add(audioTrack_1);
  AudioTrack audioTrack_2(TrackId{0});
  audioTrack_2.add(parseAudioTrackUidId("ATU_00000002"));
  transportTrackFormat_2.add(audioTrack_2);
  frame->frameHeader().add(transportTrackFormat_1);
  frame->frameHeader().add(transportTrackFormat_2);

  Combiner combiner;
  REQUIRE_THROWS_AS(combiner.push(frame), std::runtime_error);
}
