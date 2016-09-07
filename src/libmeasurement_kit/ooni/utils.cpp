// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../ooni/utils_impl.hpp"
#include "../common/utils.hpp"

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings,
               Var<Reactor> reactor, Var<Logger> logger) {
    ip_lookup_impl(callback, settings, reactor, logger);
}

void resolver_lookup(Callback<Error, std::string> callback, Settings settings,
                     Var<Reactor> reactor, Var<Logger> logger) {
    resolver_lookup_impl(callback, settings, reactor, logger);
}

/* static */ Var<IpLocationProxy> IpLocationProxy::global() {
    static Var<IpLocationProxy> singleton(new IpLocationProxy);
    return singleton;
}

Var<IpLocation> IpLocationProxy::open(std::string country, std::string asn,
                                      std::string city, Var<Logger> logger,
                                      bool *first_open) {

    // Typically we only need to refer to a single country, a single asn,
    // a single city database, hence memoize last openned instances

    if (!instance or country != instance->path_country
                  or asn != instance->path_asn
                  or city != instance->path_city) {
        logger->debug("geoip: open with '%s', '%s', '%s'", country.c_str(),
                      asn.c_str(), city.c_str());
        instance.reset(new IpLocation(country, asn, city));
        if (first_open != nullptr) {
            *first_open = true;
        }
    } else if (first_open != nullptr) {
        *first_open = false;
    }

    return instance;
}

void IpLocationProxy::close() {
    instance.reset();
}

IpLocation::IpLocation(std::string path_country_, std::string path_asn_,
                       std::string path_city_) {
    path_asn = path_asn_;
    path_country = path_country_;
    path_city = path_city_;
}

IpLocation::~IpLocation() {
    if (gi_asn != nullptr) {
        GeoIP_delete(gi_asn);
    }
    if (gi_city != nullptr) {
        GeoIP_delete(gi_city);
    }
    if (gi_country != nullptr) {
        GeoIP_delete(gi_country);
    }
}

ErrorOr<std::string> IpLocation::resolve_country_code(std::string ip,
                                                      Var<Logger> logger) {
    if (gi_country == nullptr) {
        gi_country = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_country == nullptr) {
            logger->warn("IpLocation: cannot open geoip country database");
            return CannotOpenGeoIpCountryDatabase();
        }
    }
    GeoIPLookup gl;
    memset(&gl, 0, sizeof(gl));

    const char *result;
    result = GeoIP_country_code_by_name_gl(gi_country, ip.c_str(), &gl);
    if (result == nullptr) {
        return GenericError();
    }
    std::string country_code = result;
    return country_code;
}

ErrorOr<std::string> IpLocation::resolve_country_name(std::string ip,
                                                      Var<Logger> logger) {
    if (gi_country == nullptr) {
        gi_country = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_country == nullptr) {
            logger->warn("IpLocation: cannot open geoip country database");
            return CannotOpenGeoIpCountryDatabase();
        }
    }
    GeoIPLookup gl;
    memset(&gl, 0, sizeof(gl));

    const char *result;
    result = GeoIP_country_name_by_name_gl(gi_country, ip.c_str(), &gl);
    if (result == nullptr) {
        return GenericError();
    }
    std::string country_name = result;
    return country_name;
}

ErrorOr<std::string> IpLocation::resolve_city_name(std::string ip,
                                                   Var<Logger> logger) {
    if (gi_city == nullptr) {
        gi_city = GeoIP_open(path_city.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_country == nullptr) {
            logger->warn("IpLocation: cannot open geoip city database");
            return CannotOpenGeoIpCityDatabase();
        }
    }
    GeoIPRecord *gir = GeoIP_record_by_name(gi_city, ip.c_str());
    if (gir == nullptr) {
        return GenericError();
    }
    std::string result;
    if (gir->city != nullptr) {
        result = gir->city;
    }
    GeoIPRecord_delete(gir);
    return result;
}

ErrorOr<std::string> IpLocation::resolve_asn(std::string ip,
                                             Var<Logger> logger) {
    if (gi_asn == nullptr) {
        gi_asn = GeoIP_open(path_asn.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_asn == nullptr) {
            logger->warn("IpLocation: cannot open geoip asn database");
            return CannotOpenGeoIpAsnDatabase();
        }
    }
    GeoIPLookup gl;
    memset(&gl, 0, sizeof(gl));

    char *res;
    res = GeoIP_name_by_name_gl(gi_asn, ip.c_str(), &gl);
    if (res == nullptr) {
        return GenericError();
    }
    std::string asn = res;
    asn = split(asn).front(); // We only want ASXX
    free(res);
    return asn;
}

std::string extract_html_title(std::string body) {
  std::regex TITLE_REGEXP("<title>([\\s\\S]*?)</title>", std::regex::icase);
  std::smatch match;

  if (std::regex_search(body, match, TITLE_REGEXP) && match.size() > 1) {
    return match.str(1);
  }
  return "";
}

bool is_private_ipv4_addr(const std::string &ipv4_addr) {
  std::regex IPV4_PRIV_ADDR(
      "(^127\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^192\\.168\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^10\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^172\\.1[6-9]\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^172\\.2[0-9]\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^172\\.3[0-1]\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "localhost"
  );
  std::smatch match;

  if (std::regex_search(ipv4_addr, match, IPV4_PRIV_ADDR) && match.size() > 1) {
    return true;
  }
  return false;
}

bool is_ip_addr(const std::string &ip_addr) {
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if ((inet_pton(AF_INET, ip_addr.c_str(), &(sa.sin_addr)) == 1) ||
        (inet_pton(AF_INET6, ip_addr.c_str(), &(sa6.sin6_addr)) == 1)) {
        return true;
    }
    return false;
}

} // namespace ooni
} // namespace mk
