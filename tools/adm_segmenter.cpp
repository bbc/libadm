#include <iostream>
#include <sstream>
#include "adm/common_definitions.hpp"
#include "adm/parse.hpp"
#include "adm/write.hpp"
#include "adm/segmenter.hpp"
#include "adm/combiner.hpp"
#include "submodules/CLI11.hpp"
#include <bw64/bw64.hpp>

using namespace bw64;
using namespace adm;


std::string genFilename(std::string pref, uint32_t ind);
bool readChnaFile(std::string fname, std::shared_ptr<ChnaChunk> chnaChunk);


int main(int argc, char const *argv[]) {
  CLI::App app{"ADM file segmenter"};

  std::string adm_fname, chna_fname, fout_pre;
  int size = 1000;
  int max_fr = 10;
  app.add_option("-s,--size", size, "Framesize in milliseconds");
  app.add_option("-f,--filename", adm_fname, "ADM filename")->required();
  app.add_option("-c,--chna", chna_fname, "chna filename")->required();
  app.add_option("-o,--output", fout_pre, "Output S-ADM filename prefix")->required();
  app.add_option("-n,--num", max_fr, "Maximum number of frames");
  
  CLI11_PARSE(app, argc, argv);

  auto document = parseXml(adm_fname);
  
  auto chnaChunk = std::make_shared<ChnaChunk>();
  if (!readChnaFile(chna_fname, chnaChunk)) {
    std::cerr << "No chna chnuk" << std::endl;
    exit(-1);
  }
     
  std::chrono::nanoseconds filelength((uint64_t)(max_fr * size) * 1000000L);
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
  int fr = 0;
  while (segment_start < filelength && fr < max_fr) {
    auto transportTrackFormat = segmenter.generateTransportTrackFormat(chnaChunk, 
                                                             segment_start, segment_size);   

    segmenter.addTransportTrackFormat(transportTrackFormat);
    
    auto frame = segmenter.getFrame(segment_start, segment_size);
    writeXml(xmlStream, frame);
    
    std::string fname = genFilename(fout_pre, fr);
    std::ofstream sfile(fname);

    sfile << xmlStream.str();
    xmlStream.str("");
    segment_start = SegmentStart(segment_start.get() + segment_size.get());
    fr++;
  }

  return 0;
}


std::string genFilename(std::string pref, uint32_t ind) {
  char c[256];
  sprintf(c, "%s_%05d.xml", pref.c_str(), ind);
  std::string s(c);
  return s;
}


/**
 * Reads a CHNA file and populates chnaChunk
 */
bool readChnaFile(std::string fname, std::shared_ptr<ChnaChunk> chnaChunk) {
  std::string line;
  std::ifstream fchna(fname);
  if (fchna.is_open()) {
    while (std::getline(fchna, line)) {
      std::stringstream ss(line);
      std::string word;
      std::vector<std::string> words;
      while (std::getline(ss, word, ' ')) {
        words.push_back(word);
      }
      if (words.size() == 4) {
        chnaChunk->addAudioId(AudioId(std::stoi(words[0]), words[1], words[2], words[3])); 
      }
    }
    fchna.close();
    return true;
  }
  else return false;
}
