// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NETTESTS_RUNNABLE_HPP
#define PRIVATE_NETTESTS_RUNNABLE_HPP

#include <measurement_kit/report.hpp>

#include <ctime>
#include <deque>
#include <list>
#include <sstream>

namespace mk {
namespace nettests {

class Runnable : public NonCopyable, public NonMovable {
  public:
    void begin(Callback<Error>);
    void end(Callback<Error>);

    virtual ~Runnable();

    Var<Logger> logger = Logger::make();
    Var<Reactor> reactor; /* Left unspecified on purpose */
    Settings options;
    std::list<std::string> input_filepaths;
    std::deque<std::string> inputs;
    std::string output_filepath;
    Delegate<std::string> entry_cb;
    Delegate<> begin_cb;
    std::list<Delegate<>> end_cbs;
    std::list<Delegate<>> destroy_cbs;
    bool needs_input = false;
    bool use_bouncer = true;
    std::map<std::string, std::string> test_helpers_data;

    std::string test_name = "ooni_test";
    std::string test_version = "0.0.1";
    std::string probe_ip = "127.0.0.1";
    std::string probe_asn = "AS0";
    std::string probe_cc = "ZZ";
    std::string resolver_ip = "127.0.0.1";

  protected:
    // Functions that derived classes SHOULD override
    virtual void setup(std::string);
    virtual void teardown(std::string);
    virtual void main(std::string, Settings, Callback<Var<report::Entry>>);
    virtual void fixup_entry(report::Entry &);

    // Functions that derived classes should access
    std::list<std::string> test_helpers_option_names();
    std::list<std::string> test_helpers_bouncer_names();

  private:
    report::Report report;
    tm test_start_time;
    double beginning = 0.0;

    void run_next_measurement(size_t, Callback<Error>, size_t, Var<size_t>);
    void query_bouncer(Callback<Error>);
    void geoip_lookup(Callback<>);
    void open_report(Callback<Error>);
    std::string generate_output_filepath();
};

#define MK_DECLARE_RUNNABLE(_name_)                                            \
    class _name_ : public Runnable {                                           \
      public:                                                                  \
        void main(std::string, Settings,                                       \
                  Callback<Var<report::Entry>>) override;                      \
    }

MK_DECLARE_RUNNABLE(DashRunnable);
MK_DECLARE_RUNNABLE(DnsInjectionRunnable);
MK_DECLARE_RUNNABLE(HttpHeaderFieldManipulationRunnable);
MK_DECLARE_RUNNABLE(HttpInvalidRequestLineRunnable);
MK_DECLARE_RUNNABLE(MeekFrontedRequestsRunnable);
MK_DECLARE_RUNNABLE(MultiNdtRunnable);
MK_DECLARE_RUNNABLE(NdtRunnable);
MK_DECLARE_RUNNABLE(TcpConnectRunnable);

// Separate definition because it contains extra methods
class WebConnectivityRunnable : public Runnable {
  public:
    void main(std::string, Settings, Callback<Var<report::Entry>>) override;
    void fixup_entry(report::Entry &) override;
};

} // namespace nettests
} // namespace mk
#endif
