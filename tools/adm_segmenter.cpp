#include <iostream>
#include <sstream>
#include "adm/common_definitions.hpp"
#include "adm/parse.hpp"
#include "adm/write.hpp"
#include "adm/segmenter.hpp"
#include "adm/combiner.hpp"
#include "submodules/CLI11.hpp"

int main(int argc, char const *argv[]) {
  using namespace adm;

  CLI::App app{"ADM file segmenter"};

  std::string filename;
  int size = 1000;
  app.add_option("-s,--size", size, "Framesize in milliseconds");
  app.add_option("-f,--filename", filename, "ADM filename")->required();

  CLI11_PARSE(app, argc, argv);

  auto document = parseXml(filename);
  std::chrono::nanoseconds filelength(217980000);
  for (auto programme : document->getElements<AudioProgramme>()) {
    if (programme->has<Start>() && programme->has<End>()) {
      filelength = programme->get<End>().get() - programme->get<Start>().get();
      break;
    } else if (programme->has<End>()) {
      filelength = programme->get<End>().get();
      break;
    }
  }

  std::stringstream xmlStream;
  Segmenter segmenter(document);
  SegmentStart segment_start(std::chrono::milliseconds(0));
  SegmentDuration segment_size(std::chrono::milliseconds{size});
  while (segment_start < filelength) {
    auto frame = segmenter.getFrame(segment_start, segment_size);
    writeXml(xmlStream, frame);
    std::cout << xmlStream.str();
    xmlStream.str("");
    segment_start = SegmentStart(segment_start.get() + segment_size.get());
  }

  return 0;
}
