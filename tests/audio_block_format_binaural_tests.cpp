#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#include <catch2/catch.hpp>
#include "adm/elements/audio_block_format_binaural.hpp"

TEST_CASE("audio_block_format_binaural") {
  using namespace adm;
  {
    AudioBlockFormatBinaural blockFormat;

    REQUIRE(blockFormat.has<AudioBlockFormatId>() == true);
    REQUIRE(blockFormat.has<Rtime>() == true);
    REQUIRE(blockFormat.has<Duration>() == false);
    REQUIRE(blockFormat.has<Lstart>() == false);
    REQUIRE(blockFormat.has<Lduration>() == false);
    REQUIRE(blockFormat.has<InitializeBlock>() == false);

    REQUIRE(blockFormat.isDefault<Rtime>() == true);
    REQUIRE(blockFormat.get<Rtime>().get() == std::chrono::seconds(0));

    blockFormat.set(Rtime(std::chrono::seconds(1)));
    blockFormat.set(Duration(std::chrono::seconds(10)));
    blockFormat.set(Lstart(std::chrono::seconds(0)));
    blockFormat.set(Lduration(std::chrono::seconds(10)));
    blockFormat.set(InitializeBlock(true));

    REQUIRE(blockFormat.isDefault<Rtime>() == false);
    REQUIRE(blockFormat.isDefault<Duration>() == false);

    REQUIRE(blockFormat.has<AudioBlockFormatId>() == true);
    REQUIRE(blockFormat.has<Rtime>() == true);
    REQUIRE(blockFormat.has<Duration>() == true);
    REQUIRE(blockFormat.has<Lstart>() == true);
    REQUIRE(blockFormat.has<Lduration>() == true);
    REQUIRE(blockFormat.has<InitializeBlock>() == true);

    REQUIRE(blockFormat.get<Rtime>().get() == std::chrono::seconds(1));
    REQUIRE(blockFormat.get<Duration>().get() == std::chrono::seconds(10));
    REQUIRE(blockFormat.get<Lstart>().get() == std::chrono::seconds(0));
    REQUIRE(blockFormat.get<Lduration>().get() == std::chrono::seconds(10));
    REQUIRE(blockFormat.get<InitializeBlock>() == true);

    blockFormat.unset<Rtime>();
    blockFormat.unset<Duration>();
    blockFormat.unset<Lstart>();
    blockFormat.unset<Lduration>();
    blockFormat.unset<InitializeBlock>();

    REQUIRE(blockFormat.has<Rtime>() == true);
    REQUIRE(blockFormat.has<Duration>() == false);
    REQUIRE(blockFormat.has<Lstart>() == false);
    REQUIRE(blockFormat.has<Lduration>() == false);
    REQUIRE(blockFormat.has<InitializeBlock>() == false);

    REQUIRE(blockFormat.isDefault<Rtime>() == true);

    REQUIRE(blockFormat.get<Rtime>().get() == std::chrono::seconds(0));
  }
}
