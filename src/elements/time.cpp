#include "adm/elements/time.hpp"
#include <boost/format.hpp>
#include <iomanip>
#include <regex>
#include <sstream>

namespace adm {

  std::chrono::nanoseconds parseTimecode(const std::string& timecode) {
    const static std::regex frameFormat("(\\d{2}):(\\d{2}):(\\d{2}):(\\d{2})");
    const static std::regex commonFormat(
        "(\\d{2}):(\\d{2}):(\\d{2}).(\\d{1,9})");
    const static std::regex fractionFormat("(\\d{2}):(\\d{2}):(\\d{2}).(\\d{1,6})S(\\d{1,6})");
    const static std::regex simplefractionFormat("(\\d{1,11})S(\\d{1,6})");
    std::smatch timecodeMatch;
    if (std::regex_match(timecode, timecodeMatch, commonFormat)) {
      // add trailing zeros to string
      auto nanoseconds =
          boost::str(boost::format("%1%") %
                     boost::io::group(std::setw(9), std::left,
                                      std::setfill('0'), timecodeMatch[4]));
      return std::chrono::hours(stoi(timecodeMatch[1])) +
             std::chrono::minutes(stoi(timecodeMatch[2])) +
             std::chrono::seconds(stoi(timecodeMatch[3])) +
             std::chrono::nanoseconds(stoi(nanoseconds));
    } else if (std::regex_match(timecode, timecodeMatch, frameFormat)) {
      throw std::runtime_error("frame based timecodes not supported");
    } else if (std::regex_match(timecode, timecodeMatch, fractionFormat)) {
      auto numerator =
          boost::str(boost::format("%1%") %
                     boost::io::group(std::setw(6), std::left,
                                      std::setfill('0'), timecodeMatch[4]));
      auto denominator =
          boost::str(boost::format("%1%") %
                     boost::io::group(std::setw(6), std::left,
                                      std::setfill('0'), timecodeMatch[5]));
      double frac = (double)stoi(numerator) / (double)stoi(denominator);
      uint64_t nanoseconds = (uint64_t)((double)1.0e9 * frac);
      return std::chrono::hours(stoi(timecodeMatch[1])) +
             std::chrono::minutes(stoi(timecodeMatch[2])) +
             std::chrono::seconds(stoi(timecodeMatch[3])) +
             std::chrono::nanoseconds(nanoseconds);     
    } else if (std::regex_match(timecode, timecodeMatch, simplefractionFormat)) {
      auto numerator =
          boost::str(boost::format("%1%") %
                     boost::io::group(std::setw(11), std::right,
                                      std::setfill('0'), timecodeMatch[1]));
      auto denominator =
          boost::str(boost::format("%1%") %
                     boost::io::group(std::setw(6), std::right,
                                      std::setfill('0'), timecodeMatch[2]));
      double frac = (double)stol(numerator) / (double)stol(denominator);
      uint64_t nanoseconds = (uint64_t)((double)1.0e9 * frac);
      return std::chrono::nanoseconds(nanoseconds);     
    } else {
      std::stringstream errorString;
      errorString << "invalid timecode: " << timecode;
      throw std::runtime_error(errorString.str());
    }
  }

  std::string formatTimecode(const std::chrono::nanoseconds& time) {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0')
       << std::chrono::duration_cast<std::chrono::hours>(time).count();
    ss << ":";
    ss << std::setw(2) << std::setfill('0')
       << std::chrono::duration_cast<std::chrono::minutes>(time).count() % 60;
    ss << ":";
    ss << std::setw(2) << std::setfill('0')
       << std::chrono::duration_cast<std::chrono::seconds>(time).count() % 60;
    ss << ".";
    ss << std::setw(9) << std::setfill('0') << time.count() % 1000000000;
    return ss.str();
  }

}  // namespace adm
