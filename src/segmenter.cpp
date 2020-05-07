
#include "adm/segmenter.hpp"
#include "adm/route_tracer.hpp"
#include "adm/route.hpp"
#include "adm/elements.hpp"
#include "adm/private/copy.hpp"
#include "adm/utilities/copy.hpp"
#include <algorithm>
#include <set>

#undef NDEBUG
#include <assert.h>

namespace adm {

  Segmenter::Segmenter(std::shared_ptr<Document> document)
      : document_(document) {
    // create baseFrame
    baseFrame_ = Frame::create(FrameStart(std::chrono::nanoseconds(0)),
                               FrameDuration(std::chrono::nanoseconds(0)),
                               FrameType("full"));

    for (auto channelFormat : document_->getElements<AudioChannelFormat>()) {
      sortBlockFormats<AudioBlockFormatDirectSpeakers>(channelFormat);
      sortBlockFormats<AudioBlockFormatMatrix>(channelFormat);
      sortBlockFormats<AudioBlockFormatObjects>(channelFormat);
      sortBlockFormats<AudioBlockFormatHoa>(channelFormat);
      sortBlockFormats<AudioBlockFormatBinaural>(channelFormat);
    }

    deepCopyTo(document_, baseFrame_);

    for (auto channelFormat : baseFrame_->getElements<AudioChannelFormat>()) {
      channelFormat->clearAudioBlockFormats();
    }

    RouteTracer tracer;
    for (auto audioProgramme : document_->getElements<AudioProgramme>()) {
      auto routes = tracer.run(audioProgramme);
      for (auto route : routes) {
        auto channelFormat = route.getLastOf<AudioChannelFormat>();
        auto programme = route.getFirstOf<AudioProgramme>();
        auto object = route.getLastOf<AudioObject>();
        std::chrono::nanoseconds objectStart = object->get<Start>().get();
        boost::optional<std::chrono::nanoseconds> objectEnd;
        if (object->has<Duration>()) {
          objectEnd = objectStart + object->get<Duration>().get();
        } else if (programme->has<End>()) {
          objectEnd =
              programme->get<End>().get() - programme->get<Start>().get();
        }
        detail::SegmenterItem newItem(channelFormat, objectStart, objectEnd);
        auto it = std::find_if(
            segmenterItems_.begin(), segmenterItems_.end(),
            [&newItem](const detail::SegmenterItem& item) {
              return newItem.channelFormat == item.channelFormat &&
                     newItem.start == item.start && newItem.end == item.end;
            });
        if (it == segmenterItems_.end()) {
          segmenterItems_.push_back(newItem);
        }
      }
    }
  }

  void Segmenter::addTransportTrackFormat(
      const TransportTrackFormat& transportTrackFormat) {
    baseFrame_->frameHeader().add(transportTrackFormat);
  }

  std::shared_ptr<Frame> Segmenter::getFrame(SegmentStart segStart,
                                             SegmentDuration segDuration) {
    for (auto channelFormat : baseFrame_->getElements<AudioChannelFormat>()) {
      channelFormat->clearAudioBlockFormats();
    }

    auto idValue = baseFrame_->frameHeader()
                       .frameFormat()
                       .get<FrameFormatId>()
                       .get<FrameFormatIdValue>();
    baseFrame_->frameHeader().frameFormat().set(FrameFormatId(++idValue));
    baseFrame_->frameHeader().frameFormat().set(FrameStart(segStart.get()));
    baseFrame_->frameHeader().frameFormat().set(
        FrameDuration(segDuration.get()));

    for (auto item : segmenterItems_) {
      auto channelFormatDest =
          baseFrame_->lookup(item.channelFormat->get<AudioChannelFormatId>());
      if (channelFormatDest->get<TypeDescriptor>() ==
          TypeDefinition::DIRECT_SPEAKERS) {
        addItemToChannelFormat<AudioBlockFormatDirectSpeakers>(
            item, segStart, segDuration, channelFormatDest);
      } else if (channelFormatDest->get<TypeDescriptor>() ==
                 TypeDefinition::MATRIX) {
        addItemToChannelFormat<AudioBlockFormatMatrix>(
            item, segStart, segDuration, channelFormatDest);
      } else if (channelFormatDest->get<TypeDescriptor>() ==
                 TypeDefinition::OBJECTS) {
        addItemToChannelFormat<AudioBlockFormatObjects>(
            item, segStart, segDuration, channelFormatDest);
      } else if (channelFormatDest->get<TypeDescriptor>() ==
                 TypeDefinition::HOA) {
        addItemToChannelFormat<AudioBlockFormatHoa>(item, segStart, segDuration,
                                                    channelFormatDest);
      } else if (channelFormatDest->get<TypeDescriptor>() ==
                 TypeDefinition::BINAURAL) {
        addItemToChannelFormat<AudioBlockFormatHoa>(item, segStart, segDuration,
                                                    channelFormatDest);
      }
    }
    return baseFrame_;
  }


  TransportTrackFormat Segmenter::generateTransportTrackFormat(
                                   std::shared_ptr<bw64::ChnaChunk> chnaChunk,
                                   SegmentStart segStart,
                                   SegmentDuration segDuration) {
    auto transportTrackFormat = TransportTrackFormat();
    transportTrackFormat.set(TransportId(TransportIdValue(1)));  // Assuming only one set

    std::vector<AudioTrack> audioTracks; // Vector of audioTracks to build up

    for (int i = 0; i < chnaChunk->numUids(); i++) {
      auto track_id = TrackId(chnaChunk->audioIds()[i].trackIndex());
      bool exists = false;
      for (auto &audioTrack : audioTracks) {
        if (audioTrack.get<TrackId>() == track_id) {
          auto audioTrackUidId = parseAudioTrackUidId(chnaChunk->audioIds()[i].uid());
          bool present = checkAudioObjectTimes(audioTrackUidId, segStart, segDuration);
          if (present) {
            audioTrack.add(audioTrackUidId);
          }
          exists = true; 
        }
      }
      if (!exists) {
        AudioTrack audioTrack(track_id);
        auto audioTrackUidId = parseAudioTrackUidId(chnaChunk->audioIds()[i].uid());
        bool present = checkAudioObjectTimes(audioTrackUidId, segStart, segDuration);
        if (present) {
          audioTrack.add(audioTrackUidId);
          audioTracks.push_back(audioTrack);
        }
      }
    }
    for (auto audioTrack : audioTracks) {
      transportTrackFormat.add(audioTrack);
    }

    return transportTrackFormat;
  }
  
  
  bool Segmenter::checkAudioObjectTimes(AudioTrackUidId audioTrackUidId_ref,
                                        SegmentStart segStart,
                                        SegmentDuration segDuration) {
    // Checks if the audioTrackUids in audioObjects are within the time for the segment
    bool present = true;
    for (auto audioObject : document_->getElements<AudioObject>()) {
      for (auto& audioTrackUid : audioObject->getReferences<AudioTrackUid>()) {
        if (audioTrackUidId_ref == audioTrackUid->get<AudioTrackUidId>()) {
          if (audioObject->has<Start>()) {
            auto start = audioObject->get<Start>();
            if (start.get() > segStart.get() + segDuration.get()) {
              present = false;
            }
            if (audioObject->has<Duration>()) {
              auto duration = audioObject->get<Duration>();
              if (start.get() + duration.get() <= segStart.get()) {
                present = false;
              }
            }
          }
        }
      }
    }
    return present;
  }
  
}  // namespace adm
