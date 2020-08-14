#pragma once

#include "adm/document.hpp"
#include "adm/frame.hpp"
#include "adm/route.hpp"
#include "adm/utilities/comparator.hpp"
#include <boost/range/iterator_range.hpp>
#include <chrono>
#include <map>
#include <memory>
#include <bw64/bw64.hpp>


namespace adm {

  namespace detail {
    struct SegmenterItem {
      SegmenterItem(std::shared_ptr<const AudioProgramme> programme,
                    std::shared_ptr<const AudioContent> content,
                    std::vector<std::shared_ptr<const AudioObject>> objects,
                    std::vector<std::shared_ptr<const AudioPackFormat>> packFormats,
                    std::shared_ptr<const AudioChannelFormat> channelFormat,
                    std::shared_ptr<const AudioStreamFormat> streamFormat,
                    std::shared_ptr<const AudioTrackFormat> trackFormat,
                    std::shared_ptr<const AudioTrackUid> trackUid,
                    std::chrono::nanoseconds start,
                    boost::optional<std::chrono::nanoseconds> end)
          : programme(programme),
            content(content),
            objects(objects),
            packFormats(packFormats),
            channelFormat(channelFormat),
            streamFormat(streamFormat),
            trackFormat(trackFormat),
            trackUid(trackUid),
            start(start), end(end){};
      std::shared_ptr<const AudioProgramme> programme;
      std::shared_ptr<const AudioContent> content;
      std::vector<std::shared_ptr<const AudioObject>> objects;
      std::vector<std::shared_ptr<const AudioPackFormat>> packFormats;
      std::shared_ptr<const AudioChannelFormat> channelFormat;
      std::shared_ptr<const AudioStreamFormat> streamFormat;
      std::shared_ptr<const AudioTrackFormat> trackFormat;
      std::shared_ptr<const AudioTrackUid> trackUid;
      std::chrono::nanoseconds start;
      boost::optional<std::chrono::nanoseconds> end;
      bool use;
    };
  }  // namespace detail

  /// @brief Tag for NamedType ::SegmentStart
  struct SegmentStartTag {};
  /// @brief NamedType for the audioContentName attribute
  using SegmentStart =
      detail::NamedType<std::chrono::nanoseconds, SegmentStartTag>;
  /// @brief Tag for NamedType ::SegmentDuration
  struct SegmentDurationTag {};
  /// @brief NamedType for the audioContentName attribute
  using SegmentDuration =
      detail::NamedType<std::chrono::nanoseconds, SegmentDurationTag>;

  /**
   * @brief Segment Document into Frames
   *
   * For now this class can only create full frames.
   *
   * The segmenter adds all AudioBlockFormats which *could* be relevant to the
   * requested segment. It does take into account the duration of the
   * AudioObject and AudioProgramme, but ignores the duration of the
   * AudioBlockFormat. Consider the following example:
   *
   * @code
   0s    1s    2s    3s    4s    5s    6s
   |-----|-----|-----|-----|-----|-----|

         +-----+-----+-----+-----+
         | AO1                   |
         +-----+-----+-----+-----+
         | AB1 | AB2 | AB3 | AB4 |
         +-----+-----+-----+-----+
   @endcode
   *
   * For the following segment ranges the behaviour will be as follows:
   *
   *  - [0s; 0.5s]: no AudioBlockFormat
   *  - [1s; 2s]: AB1, AB2
   *    - AB2 to calc the duration of AB1
   *  - [2s; 2.5s]: AB1, AB2, AB3
   *    - AB1 to calc the start values of an interpolation
   *    - AB2 to calc the end values of an interpolation
   *    - AB3 to calc the duration of AB2
   *  - [2.25s; 2.75s]: AB1, AB2, AB3
   *    - AB1 to calc the start values of an interpolation
   *    - AB2 to calc the end values of an interpolation
   *    - AB3 to calc the duration of AB2
   *  - [2s; 3s]: AB1, AB2, AB3
   *    - AB1 to calc the start values of an interpolation
   *    - AB2 to calc the end values of an interpolation
   *    - AB3 to calc the duration of AB2
   *    - AB4 is not relevant yet!
   *  - [4.5s; 5s]: AB3, AB4
   *    - AB3 to calc the start values of an interpolation
   *    - AB4 to calc the end values of an interpolation
   */
  class Segmenter {
   public:
    /**
     * @brief Segmenter Ctor
     *
     * @warning All the audioBlockFormats in the adm::document, which is passed
     * to the ctor will be sorted by time!
     */
    ADM_EXPORT Segmenter(std::shared_ptr<Document> document);

    /**
     * @brief add a TransportTrackFormat
     *
     * This TransportTrackFormat will be present in all generated/segmented
     * `Frame`s.
     */
    ADM_EXPORT void addTransportTrackFormat(
        const TransportTrackFormat& transportTrackFormat);
    
    /**
     * @brief Get a Frame with the given interval
     *
     * With every call the FrameFormatId is incremented.
     *
     * Similar to the Start and Duration of an AudioObject SegmentStart and
     * SegmentDuration are relative to the Start of the file.
     */
    ADM_EXPORT std::shared_ptr<Frame> getFrame(SegmentStart start,
                                               SegmentDuration segDuration);
    
    /**
     * @brief Generates the transportTrackFormat element 
     *
     * Uses the chna chunk to get the information on the UIDs allocated to each track.
     */
    ADM_EXPORT TransportTrackFormat generateTransportTrackFormat(
                                   std::shared_ptr<bw64::ChnaChunk> chnaChunk,
                                   SegmentStart segStart,
                                   SegmentDuration segDuration);
      
      
    /**
     * @brief Checks if audioTrackUID is present this frame
     *
     * Checks if the given audioTrackUidId_ref is present in the specified segment time
     * using the audioObject start and duration that references it.
     */
    ADM_EXPORT bool checkAudioObjectTimes(AudioTrackUidId audioTrackUidId_ref,
                            SegmentStart segStart,
                            SegmentDuration segDuration);
    
    
   private:
    void fixReferences();
    
    std::shared_ptr<Document> document_;
    std::shared_ptr<Frame> baseFrame_;
    std::vector<detail::SegmenterItem> segmenterItems_;
  };

  // ---- Implementation ---- //

  // FIXME: Make private or move to detail !!!

  /**
   * @brief Add relevant AudioBlocks for the given timespan to
   * AudioChannelFormat
   */
  template <typename AudioBlockFormat>
  uint32_t addItemToChannelFormat(const detail::SegmenterItem& item,
                              SegmentStart segStart,
                              SegmentDuration segDuration,
                              std::shared_ptr<AudioChannelFormat> dest) {
    auto foundBlockFormats =
        findBlockFormats(item.channelFormat->getElements<AudioBlockFormat>(),
                         segStart, segDuration, item.start, item.end);
    
    // Add valid blocks to destination channel
    uint32_t numBlocks = 0;
    for (auto blockFormat : foundBlockFormats) {
      dest->add(blockFormat);
      numBlocks++;
    }

    return numBlocks;
  }

  /**
   * @brief Find relevant AudioBlocks for the given timespan
   *
   * NOTE: The AudioBlockFormatRange has to be sorted.
   */
  template <typename AudioBlockFormatRange>
  boost::iterator_range<typename AudioBlockFormatRange::iterator>
  findBlockFormats(AudioBlockFormatRange blockFormatSrc, SegmentStart segStart,
                   SegmentDuration segDuration,
                   std::chrono::nanoseconds objectStart,
                   boost::optional<std::chrono::nanoseconds> objectEnd) {
    typedef typename AudioBlockFormatRange::value_type AudioBlockFormat;
    
    // object already ended before segment start -> return no blocks
    if (objectEnd && objectEnd < segStart.get()) {
      return boost::make_iterator_range(blockFormatSrc.end(),
                                        blockFormatSrc.end());
    }

    // object did not begin before segment end -> return no blocks
    if (objectStart > segStart.get() + segDuration.get()) {
      return boost::make_iterator_range(blockFormatSrc.end(),
                                        blockFormatSrc.end());
    }

    // shrink search range to object bounds
    std::chrono::nanoseconds effectiveStart = segStart.get();
    if (effectiveStart < objectStart) {
      effectiveStart = objectStart;
    }
    std::chrono::nanoseconds effectiveEnd = segStart.get() + segDuration.get();
    if (objectEnd && objectEnd.get() < effectiveEnd) {
      effectiveEnd = objectEnd.get();
    }

    // Find the first and last valid blocks
    auto begin = blockFormatSrc.end();
    auto end = blockFormatSrc.begin();
    for (auto it = blockFormatSrc.begin() ; it != blockFormatSrc.end(); it++) {
      auto blockFormat = *it;
      auto seg_end = segStart.get() + segDuration.get();
      auto block_start = objectStart + blockFormat.template get<Rtime>().get();
      auto block_end = seg_end;
      if (blockFormat.template has<Duration>()) {
        block_end = objectStart + blockFormat.template get<Rtime>().get() + blockFormat.template get<Duration>().get();
      }

      if (block_start < seg_end && block_end > segStart.get()) {
        if (begin == blockFormatSrc.end()) {
          begin = it;
        }
        end = it;
      }
    }
    // If last block has been set move past one to end
    if (end != blockFormatSrc.begin()) {
      end++;
    } else {  
      // Otherwise set to end of iterator
      end = blockFormatSrc.end();
    }

    return boost::make_iterator_range(begin, end);
  }      
    
  template <typename AudioBlockFormat>
  void sortBlockFormats(std::shared_ptr<AudioChannelFormat> channelFormat) {
    auto blockFormats = channelFormat->getElements<AudioBlockFormat>();
    std::sort(blockFormats.begin(), blockFormats.end(),
              CompareRtimeDurationLess());
  }

}  // namespace adm
