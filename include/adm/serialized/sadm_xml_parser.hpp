#pragma once
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "adm/document.hpp"
#include "adm/elements.hpp"
#include "adm/errors.hpp"
#include "adm/parse_sadm.hpp"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"

#include "adm/serialized/frame_format.hpp"
#include "adm/serialized/frame_format_id.hpp"
//#include "adm/serialized/flow_id.hpp"
#include "adm/serialized/transport_track_format.hpp"
#include "adm/serialized/transport_id.hpp"
#include "adm/serialized/audio_track.hpp"
#include "adm/frame.hpp"
//#include "adm/private/xml_parser.hpp"
//#include "adm/private/xml_parser_helper.hpp"


#include <iostream>

namespace adm {
  /**
   * @brief XML parsing implementation
   *
   * This namespace collects parser implementations and helpers for writing
   * parsers.
   *
   * If you're just looking for a way to parse an XML file in client code,
   * refer to `adm::parseXml()`
   */
  namespace xml {
    using NodePtr = rapidxml::xml_node<>*;
        
    class SadmXmlParser {
     public:
      SadmXmlParser(std::istream& stream,
                    ParserOptions options = ParserOptions::none,
                    std::shared_ptr<Frame> destFrame = Frame::create(FrameStart(std::chrono::milliseconds(0)),
                               FrameDuration(std::chrono::milliseconds(1000)),
                               FrameType("full")));

      std::shared_ptr<Frame> parse();

     private:
      NodePtr findFrameNode(NodePtr root);

      FrameHeader parseFrameHeader(NodePtr node);

      std::shared_ptr<AudioProgramme> parseAudioProgramme(NodePtr node);
      std::shared_ptr<AudioContent> parseAudioContent(NodePtr node);
      std::shared_ptr<AudioObject> parseAudioObject(NodePtr node);
      std::shared_ptr<AudioTrackFormat> parseAudioTrackFormat(NodePtr node);
      std::shared_ptr<AudioStreamFormat> parseAudioStreamFormat(NodePtr node);
      std::shared_ptr<AudioPackFormat> parseAudioPackFormat(NodePtr node);
      std::shared_ptr<AudioTrackUid> parseAudioTrackUid(NodePtr node);
      std::shared_ptr<AudioChannelFormat> parseAudioChannelFormat(NodePtr node);

      rapidxml::file<> xmlFile_;
      std::shared_ptr<Frame> frame_;
      ParserOptions options_;
      
      // clang-format off
      std::map<std::shared_ptr<AudioProgramme>, std::vector<AudioContentId>> programmeContentRefs_;
      std::map<std::shared_ptr<AudioContent>, std::vector<AudioObjectId>> contentObjectRefs_;
      std::map<std::shared_ptr<AudioObject>, std::vector<AudioObjectId>> objectObjectRefs_;
      std::map<std::shared_ptr<AudioObject>, std::vector<AudioPackFormatId>> objectPackFormatRefs_;
      std::map<std::shared_ptr<AudioObject>, std::vector<AudioTrackUidId>> objectTrackUidRefs_;
      std::map<std::shared_ptr<AudioTrackUid>, AudioTrackFormatId> trackUidTrackFormatRef_;
      std::map<std::shared_ptr<AudioTrackUid>, AudioPackFormatId> trackUidPackFormatRef_;
      std::map<std::shared_ptr<AudioTrackUid>, AudioChannelFormatId> trackUidChannelFormatRef_;
      std::map<std::shared_ptr<AudioPackFormat>, std::vector<AudioChannelFormatId>> packFormatChannelFormatRefs_;
      std::map<std::shared_ptr<AudioPackFormat>, std::vector<AudioPackFormatId>> packFormatPackFormatRefs_;
      std::map<std::shared_ptr<AudioTrackFormat>, AudioStreamFormatId> trackFormatStreamFormatRef_;
      std::map<std::shared_ptr<AudioStreamFormat>, AudioChannelFormatId> streamFormatChannelFormatRef_;
      std::map<std::shared_ptr<AudioStreamFormat>, AudioPackFormatId> streamFormatPackFormatRef_;
      std::map<std::shared_ptr<AudioStreamFormat>, std::vector<AudioTrackFormatId>> streamFormatTrackFormatRefs_;
      
      std::map<std::shared_ptr<AudioTrack>, std::vector<AudioTrackUidId>> trackTrackUidRefs_;
      // clang-format on

      template <typename Src, typename TargetId>
      void resolveReferences(std::map<Src, std::vector<TargetId>> map) {
        for (auto entry : map) {
          for (auto id : entry.second) {
            if (frame_->lookup(id) != nullptr) {
              entry.first->addReference(frame_->lookup(id));
            } else {
              throw error::XmlParsingUnresolvedReference(formatId(id));
            }
          }
        }
      }

      template <typename Src, typename Target>
      void resolveReference(std::map<Src, Target> map) {
        for (auto entry : map) {
          auto id = entry.second;
          if (auto element = frame_->lookup(id)) {
            entry.first->setReference(element);
          } else {
            throw error::XmlParsingUnresolvedReference(formatId(id));
          }
        }
      }
      
      void setCommonProperties(std::shared_ptr<AudioPackFormat> audioPackFormat,
                               NodePtr node);
    };

  }  // namespace xml
}  // namespace adm
