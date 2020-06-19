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


std::string genSadmFilename(std::string pref, uint32_t ind);
std::string genWavFilename(std::string pref, uint32_t ind);


int main(int argc, char const *argv[]) {
  CLI::App app{"BW64 file segmenter"};

  std::string bw64_fname, fout_pre;
  int size = 1000;
  int max_fr = 10;
  app.add_option("-s,--size", size, "Framesize in milliseconds");
  app.add_option("-f,--filename", bw64_fname, "BW64 filename")->required();
  app.add_option("-o,--output", fout_pre, "Output S-ADM/WAV filename prefix")->required();
  app.add_option("-n,--num", max_fr, "Maximum number of frames");
  
  CLI11_PARSE(app, argc, argv);

  auto bw64File = readFile(bw64_fname);
  if (!bw64File->axmlChunk()) {
    std::cerr << "No AXML chunk in input file" << std::endl;
    exit(-1);
  }
  
  auto axmlChunk = bw64File->axmlChunk();  // get axml chunk
  auto chnaChunk = bw64File->chnaChunk();  // get chna chunk

  // Extract the ADM metadata
  std::stringstream adm_data(""); 
  axmlChunk->write(adm_data);
  auto document = parseXml(adm_data);
     
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
    
    std::string fname = genSadmFilename(fout_pre, fr);
    std::ofstream sfile(fname);

    sfile << xmlStream.str();
    
    // Generate wav file for frame
    fname = genWavFilename(fout_pre, fr);
    auto wavfile = writeFile(fname, bw64File->channels(), bw64File->sampleRate(),
                           bw64File->bitDepth());
    
    // Read and write sample
    uint32_t frame_size = (uint32_t)((double)size * (double)bw64File->sampleRate() / 1000.0);
    std::vector<float> ibuffer(frame_size * bw64File->channels());
    if (!bw64File->eof()) {
      auto readFrames = bw64File->read(&ibuffer[0], frame_size);
      wavfile->write(&ibuffer[0], readFrames);
    }
    
    // Get ready for next frame
    xmlStream.str("");
    segment_start = SegmentStart(segment_start.get() + segment_size.get());
    fr++;
  }

  return 0;
}


std::string genSadmFilename(std::string pref, uint32_t ind) {
  char c[256];
  sprintf(c, "%s_%05d.xml", pref.c_str(), ind);
  std::string s(c);
  return s;
}


std::string genWavFilename(std::string pref, uint32_t ind) {
  char c[256];
  sprintf(c, "%s_%05d.wav", pref.c_str(), ind);
  std::string s(c);
  return s;
}


