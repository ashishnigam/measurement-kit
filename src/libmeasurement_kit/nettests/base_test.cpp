// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/worker.hpp"
#include "private/nettests/runnable.hpp"
#include "private/nettests/runner.hpp"

#include <measurement_kit/nettests.hpp>

#include <cassert>

namespace mk {
namespace nettests {

BaseTest &BaseTest::on_logger_eof(Delegate<> func) {
    runnable->logger->on_eof(func);
    return *this;
}

BaseTest &BaseTest::on_log(Delegate<uint32_t, const char *> func) {
    runnable->logger->on_log(func);
    return *this;
}

BaseTest &BaseTest::on_event(Delegate<const char *> func) {
    runnable->logger->on_event(func);
    return *this;
}

BaseTest &BaseTest::on_progress(Delegate<double, const char *> func) {
    runnable->logger->on_progress(func);
    return *this;
}

BaseTest &BaseTest::set_verbosity(uint32_t level) {
    runnable->logger->set_verbosity(level);
    return *this;
}

BaseTest &BaseTest::increase_verbosity() {
    runnable->logger->increase_verbosity();
    return *this;
}

BaseTest::BaseTest() {}
BaseTest::~BaseTest() {}

BaseTest &BaseTest::add_input(std::string s) {
    // Note: ooni-probe does not allow to specify more than one input from the
    // command line. Given that the underlying code allows that, I do not see a
    // reason to be artifically restrictive here.
    runnable->inputs.push_back(s);
    return *this;
}

BaseTest &BaseTest::add_input_filepath(std::string s) {
    runnable->input_filepaths.push_back(s);
    return *this;
}

BaseTest &BaseTest::set_input_filepath(std::string s) {
    runnable->input_filepaths.clear();
    return add_input_filepath(s);
}

BaseTest &BaseTest::set_output_filepath(std::string s) {
    runnable->output_filepath = s;
    return *this;
}

BaseTest &BaseTest::set_error_filepath(std::string s) {
    runnable->logger->set_logfile(s);
    return *this;
}

BaseTest &BaseTest::set_options(std::string key, std::string value) {
    runnable->options[key] = value;
    return *this;
}

BaseTest &BaseTest::on_entry(Delegate<std::string> cb) {
    runnable->entry_cb = cb;
    return *this;
}

BaseTest &BaseTest::on_begin(Delegate<> cb) {
    runnable->begin_cb = cb;
    return *this;
}

BaseTest &BaseTest::on_end(Delegate<> cb) {
    runnable->end_cbs.push_back(cb);
    return *this;
}

BaseTest &BaseTest::on_destroy(Delegate<> cb) {
    runnable->destroy_cbs.push_back(cb);
    return *this;
}

static void start_internal_(Var<Runnable> &&r, std::promise<void> *promise,
                            Callback<> &&callback) {
    assert(!r->reactor);
    Worker::default_tasks_queue()->run_in_background_thread(
          [ r = std::move(r), promise, callback = std::move(callback) ] {
              r->reactor = Reactor::make();
              r->reactor->run_with_initial_event([&]() {
                  r->begin([&](Error) {
                      r->end([&](Error) {
                          r->reactor->stop();
                          callback();
                      });
                  });
              });
              if (promise != nullptr) {
                  promise->set_value();
              }
          });
}

void BaseTest::run() {
    // Note:
    //
    // 1. runnable->reactor SHOULD NOT be specified: we pick the best one
    //    depending on what we need to do.
    //
    // 2. in this case, we create a reactor in the stack and use it. We do
    //    this such that we're sure it's a good reactor that works (and not
    //    one that previosuly failed and cannot enter the loop).
    //
    // 3. the test lifecycle is guaranteed by the fact that it's created
    //    on the stack and reactor->loop...() is blocking.
#if 0
    assert(!runnable->reactor);
    Var<Reactor> reactor = Reactor::make();
    runnable->reactor = reactor;
    reactor->loop_with_initial_event([&]() {
        runnable->begin([&](Error) {
            runnable->end([&](Error) {
                reactor->call_soon([&]() {
                    reactor->stop();
                });
            });
        });
    });
#endif
    std::promise<void> promise;
    std::future<void> future = promise.get_future();
    start_internal_(std::move(runnable), &promise, []() {});
    future.get();

}

void BaseTest::start(Callback<> callback) {
    // Note:
    //
    // 1. runnable->reactor SHOULD NOT be specified: we pick the best one
    //    depending on what we need to do.
    //
    // 2. in this case, we run in a background thread, possibly waiting
    //    inside a queue until it's out turn, and we are assigned a known
    //    good reactor. Specifically, when the lambda passed to run...
    //    is called, the reactor is already running.
    //
    // 3. the test lifecycle is guaranteed by the fact that we register
    //    an end-of-loop cleanup for the test itself: this guarantees
    //    that the test main object will be alive for _as long as_ the
    //    reactor loop (and possibly more).
    //
    // 4. after specific changes, now the reactor runs as long as we
    //    have I/O events, pending calls, running threads. Since we are
    //    using a single reactor per test, the implication is that we
    //    will leave the event loop when all the I/O, calls, and threads
    //    generated by the test are terminated.
    //
    // 5. using std::swap we invalidate the test structure such that
    //    attempting to start() the test again will not work.
    start_internal_(std::move(runnable), nullptr, std::move(callback));
}

} // namespace nettests
} // namespace mk
