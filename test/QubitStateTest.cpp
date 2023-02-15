//
// Created by zuchna on 2/4/23.
//

#include "TestUtils.hpp"

TEST_CASE("QubitState normalization example") {
    Complex c1(1, 0);
    Complex c2(1, 0);

    QubitState qs(1);
    qs[BitSet(1, 0)] = c1;
    qs[BitSet(1, 1)] = c2;

    qs.normalize();
    REQUIRE(qs.norm() == Catch::Approx(1.0));
}

TEST_CASE("QubitState normalization random test") {
    //Generate 4*4*4*4 = 256 doubles
    auto p1 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p2 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p3 = GENERATE(take(4, random(-10.0, 10.0)));
    auto p4 = GENERATE(take(4, random(-10.0, 10.0)));

    auto c1 = Complex(p1, p2);
    auto c2 = Complex(p3, p4);

    QubitState qs(1);
    qs[BitSet(1, 0)] = c1;
    qs[BitSet(1, 1)] = c2;

    qs.normalize();

    REQUIRE(qs.norm() == Catch::Approx(1.0));
}

TEST_CASE("QubitState Merge Example") {
    //Build Register for testing
    /*
     * 0&2: |00> = 1/sqrt(2), |11> = 1/sqrt(2)
     * 1: |0> = 0.5, |1> = sqrt(3)/2
     */

    QubitStateOrTop testQReg[3]{
            std::make_shared<QubitState>(2),
            std::make_shared<QubitState>(1),
            TOP::T
    };

    (testQReg[0]).getQubitState()->clear();
    (*testQReg[0].getQubitState())[BitSet(2, 0)] = Complex(0.70710678118, 0);
    (*testQReg[0].getQubitState())[BitSet(2, 3)] = Complex(0.70710678118, 0);
    (*testQReg[1].getQubitState())[BitSet(1, 0)] = Complex(0.5, 0);
    (*testQReg[1].getQubitState())[BitSet(1, 1)] = Complex(0.86602540378, 0);

    testQReg[2] = testQReg[0].getQubitState();

    std::shared_ptr<QubitState> q0_2 = testQReg[0].getQubitState();
    std::shared_ptr<QubitState> q1 = testQReg[1].getQubitState();


    std::shared_ptr<QubitState> result = QubitState::combine(q0_2, std::vector{0, 2}, q1, std::vector{1});

    Complex expected0 = Complex(qc::SQRT_2_2 / 2.0, 0);
    Complex expected2 = Complex(qc::SQRT_3 * qc::SQRT_2_2 / 2.0, 0);
    Complex expected5 = expected0;
    Complex expected7 = expected2;

    //Make sure my solution is at least normed
    double sum = expected0.norm() + expected2.norm() + expected5.norm() + expected7.norm();
    REQUIRE(Complex(sum) == Complex(1, 0));

    //Check if the result is correct
    REQUIRE(result->size() == 4);

    REQUIRE((*result)[BitSet(3, 0)] == expected0);

    REQUIRE((*result)[BitSet(3, 2)] == expected2);

    REQUIRE((*result)[BitSet(3, 5)] == expected5);

    REQUIRE((*result)[BitSet(3, 7)] == expected7);
}

TEST_CASE("Test Gate Identities") {
    std::array<Complex, 4> X = {0, 1, 1, 0};
    std::array<Complex, 4> I = {1, 0, 0, 1};

    std::array<Complex, 4> Y = {0, Complex(0, -1), Complex(0, 1), 0};

    std::array<Complex, 4> Z = {1, 0, 0, -1};

    std::array<Complex, 4> H = {
            Complex(1 / qc::SQRT_2, 0), Complex(1 / qc::SQRT_2, 0),
            Complex(1 / qc::SQRT_2, 0), Complex(-1 / qc::SQRT_2, 0)
    };

    std::array<Complex, 4> S = {
            Complex(1, 0), Complex(0, 0),
            Complex(0, 0), Complex(0, 1)
    };

    std::array<Complex, 4> T = {
            Complex(1, 0), Complex(0, 0),
            Complex(0, 0), Complex(1 / qc::SQRT_2, 1 / qc::SQRT_2)
    };

    auto nQubits = GENERATE(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    QubitState qs(nQubits);
    qs.clear();

    for (int key = 1 << (nQubits - 1); key >= 0; key--) {
        //Generate Amplitudes for 2/3 of keys
        if (rand() % 3 > 0) {
            qs[BitSet(nQubits, key)] = Complex(rand() % 100, rand() % 100);
        } else {
            continue;
        }
    }

    qs.normalize();
    QubitState result(0);

    for (int target = 0; target < nQubits; target++) {

        //Use Relationships to verify

        //I = I
        result = qs.clone();
        result.applyGate(target, I);
        CHECK_MESSAGE(qs == result,
                      "I = I Failed, nQubits: " + std::to_string(nQubits) + ", target: " + std::to_string(target) +
                      ", qs: " + qs.to_string() + ", result: " + result.to_string());

        //XX = I
        result = qs.clone();
        result.applyGate(target, X);
        result.applyGate(target, X);
        CHECK_MESSAGE(qs == result,
                      "XX = I Failed, nQubits: " + std::to_string(nQubits) + ", target: " + std::to_string(target) +
                      ", qs: " + qs.to_string() + ", result: " + result.to_string());

        //HH = I
        result = qs.clone();
        result.applyGate(target, H);
        result.applyGate(target, H);
        CHECK_MESSAGE(qs == result,
                      "HH = I Failed, nQubits: " + std::to_string(nQubits) + ", target: " + std::to_string(target) +
                      ", qs: " + qs.to_string() + ", result: " + result.to_string());

        //ZZ = I
        result = qs.clone();
        result.applyGate(target, Z);
        result.applyGate(target, Z);
        CHECK_MESSAGE(qs == result,
                      "ZZ = I Failed, nQubits: " + std::to_string(nQubits) + ", target: " + std::to_string(target) +
                      ", qs: " + qs.to_string() + ", result: " + result.to_string());

        //YY = I
        result = qs.clone();
        result.applyGate(target, Y);
        result.applyGate(target, Y);
        CHECK_MESSAGE(qs == result,
                      "YY = I Failed, nQubits: " + std::to_string(nQubits) + ", target: " + std::to_string(target) +
                      ", qs: " + qs.to_string() + ", result: " + result.to_string());
    }
}