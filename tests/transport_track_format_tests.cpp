#include <catch2/catch.hpp>
#include "adm/serialized/frame_header.hpp"
#include "adm/serialized/transport_track_format.hpp"
#include "adm/serialized/transport_track_format_id.hpp"

TEST_CASE("transport_track_format") {
  using namespace adm;
  // Attributes / Elements
  {
    TransportTrackFormat transportTrackFormat;

    REQUIRE(transportTrackFormat.has<TransportId>() == true);
    REQUIRE(transportTrackFormat.has<TransportName>() == false);
    REQUIRE(transportTrackFormat.has<NumTracks>() == true);
    REQUIRE(transportTrackFormat.has<NumIds>() == true);

    transportTrackFormat.set(TransportName("MyTransportTrackFormat"));
    REQUIRE(transportTrackFormat.has<TransportName>() == true);

    REQUIRE(transportTrackFormat.get<TransportId>().get<TransportIdValue>() ==
            0u);
    REQUIRE(transportTrackFormat.get<TransportName>() ==
            "MyTransportTrackFormat");
    REQUIRE(transportTrackFormat.get<NumTracks>().get() == 0u);
    REQUIRE(transportTrackFormat.get<NumIds>().get() == 0u);

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

    REQUIRE(transportTrackFormat.get<NumTracks>().get() == 3u);
    REQUIRE(transportTrackFormat.get<NumIds>().get() == 4u);

    transportTrackFormat.unset<TransportName>();
    transportTrackFormat.clearAudioTracks();

    REQUIRE(transportTrackFormat.has<TransportName>() == false);
    REQUIRE(transportTrackFormat.get<NumTracks>().get() == 0u);
    REQUIRE(transportTrackFormat.get<NumIds>().get() == 0u);
  }
}
