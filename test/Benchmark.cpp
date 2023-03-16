#include "TestUtils.hpp"
#include "CircuitOptimizer.hpp"
#include <chrono>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/catch_test_case_info.hpp>


class testRunListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const &info) override {
        if (info.name == "Test Circuit Performance") {
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_c);

            char buf[80];
            std::setlocale(LC_TIME, "C");
            std::strftime(buf, 80, "%H:%M:%S", &now_tm);

            std::cout << "Test Started at: " << buf << std::endl;
            std::cout
                    << "fileName,maxNAmpls,parseTime,nQubits,nOpsStart,flattenTime,nOpsAfterInline,propagateTime,nOpsAfterPropagate,wasTop"
                    << std::endl;
        }
    }
};

CATCH_REGISTER_LISTENER(testRunListener);

namespace c = std::chrono;
using tp = std::chrono::steady_clock::time_point;

//Took about 1:40h in my Laptop
//Run this with ./tests [!benchmark] --reporter console
//to copy std::cout to csv file
TEST_CASE("Test Circuit Performance", "[!benchmark]") {
    fs::path file = GENERATE(take(250, qasmFile(QASMFileGenerator::ALL)));
    size_t maxNAmpls = 1024;
    size_t eps = 0.0;

    tp start, end;
    long long dur;

    //commit,fileName,maxNAmpls,epsolon
    std::cout << GIT_COMMIT_HASH << file.string() << "," << maxNAmpls << "," << eps;
    std::cout.flush();

    start = c::steady_clock::now();
    qc::QuantumComputation qc;
    try {
        qc = qc::QuantumComputation(file.string());
    } catch (std::exception &e) {
        std::cout << ", qfr threw an exception while importing: " << e.what() << std::endl;
        return;
    }

    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //parseTime,nQubits,nOpsStart
    std::cout << "," << dur << "," << qc.getNqubits() << "," << qc.getNops();
    std::cout.flush();

    start = c::steady_clock::now();
    qc::CircuitOptimizer::flattenOperations(qc);
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //flattenTime,nOpsAfterInline
    std::cout << "," << dur << "," << qc.getNops();
    std::cout.flush();

    start = c::steady_clock::now();
    auto ut = ConstantPropagation::propagate(qc, maxNAmpls);
    end = c::steady_clock::now();
    dur = c::duration_cast<c::microseconds>(end - start).count();

    //propagateTime,nOpsAfterPropagate
    std::cout << "," << dur << "," << qc.getNops();

    size_t wasTop = 0;
    for (auto &qs: *ut) {
        if (qs.isTop()) {
            wasTop++;
        }
    }

    //wasTop
    std::cout << "," << wasTop << std::endl;
}
