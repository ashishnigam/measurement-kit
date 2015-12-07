// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_HTTP_ERROR_HPP
#define MEASUREMENT_KIT_HTTP_ERROR_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {
namespace http {

/// Received UPGRADE request error
class UpgradeError : public Error {
  public:
    UpgradeError (): Error(3000, "unknown_error 3000") {};
};

/// Parse error occurred
class ParserError : public Error {
  public:
    ParserError() : Error(3001, "unknown_error 3001") {};
};

} // namespace http
} // namespace mk
#endif