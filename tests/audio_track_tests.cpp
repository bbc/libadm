#include <catch2/catch.hpp>
#include "adm/elements/audio_track_uid.hpp"
#include "adm/serialized/audio_track.hpp"

TEST_CASE("audio_track") {
  using namespace adm;
  // Attributes / Elements
  {
    AudioTrack audioTrack(TrackId(1));

    REQUIRE(audioTrack.get<TrackId>() == 1u);
    REQUIRE(audioTrack.has<FormatDescriptor>() == false);

    audioTrack.set(FormatDescriptor(FormatDefinition::PCM));
    REQUIRE(audioTrack.has<FormatDescriptor>() == true);
    REQUIRE(audioTrack.get<FormatDescriptor>() == FormatDefinition::PCM);

    audioTrack.unset<FormatDescriptor>();

    REQUIRE(audioTrack.has<FormatDescriptor>() == false);

    // add AudioTrackUidIds
    audioTrack.add(parseAudioTrackUidId("ATU_00000001"));
    audioTrack.add(parseAudioTrackUidId("ATU_00000002"));

    REQUIRE(audioTrack.audioTrackUidIds().size() == 2);
  }
}
