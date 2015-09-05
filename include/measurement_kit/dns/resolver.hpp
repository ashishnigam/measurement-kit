// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_RESOLVER_HPP
#define MEASUREMENT_KIT_DNS_RESOLVER_HPP

#include <measurement_kit/common/constraints.hpp>
#include <measurement_kit/common/libs.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/settings.hpp>

#include <functional>
#include <iosfwd>
#include <string>

struct evdns_base; // Internally we use evdns

namespace measurement_kit {
namespace dns {

class Response;

using namespace measurement_kit::common;

/// DNS Resolver object.
class Resolver : public NonCopyable, public NonMovable {

  private:
    void cleanup();

  protected:
    Settings settings;
    Libs *libs = Libs::global();
    Poller *poller = measurement_kit::get_global_poller();
    evdns_base *base = nullptr;
    Logger *logger = Logger::global();

  public:
    /// Default constructor.
    Resolver() {}

    /// Constructor with specific settings.
    Resolver(Settings settings_, Logger *lp = Logger::global(),
             Libs *lev = nullptr, Poller *plr = nullptr) {
        if (lev != nullptr) {
            libs = lev;
        }
        if (plr != nullptr) {
            poller = plr;
        }
        settings = settings_;
        logger = lp;
    }

    /// Get the evdns_base bound to the settings.
    evdns_base *get_evdns_base();

    /// Issue a Query using this resolver.
    void query(std::string query, std::string address,
               std::function<void(Response &&)> &&func);

    /// Default destructor.
    ~Resolver() { cleanup(); }
};

} // namespace dns
} // namespace measurement_kit
#endif