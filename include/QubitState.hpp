//
// Created by Jakob on 31/01/2023.
//

#ifndef QCPROP_QUBITSTATE_HPP
#define QCPROP_QUBITSTATE_HPP


#include <map>
#include <complex>
#include <bitset>
#include "util/Complex.hpp"
#include <iostream>
#include "util/BitSet.hpp"
#include "operations/OpType.hpp"

class QubitState {
public:
    explicit QubitState(size_t nQubits);

    QubitState(const QubitState &qubitState) = default;

    ~QubitState() = default;

    [[nodiscard]] size_t size() const;

    [[nodiscard]] size_t getNQubits() const;

    void clear() {
        this->map.clear();
    }

    QubitState clone() const;

    void print(std::ostream &os) const;

    //Iterator over map entries
    typedef std::map<BitSet, Complex>::iterator iterator;

    /**
     * Combine Quantum States by applying the tensor product
     * New keys are generated by sorting the indices in ascending order
     * LSB of the Key is smallest index
     * @param qubitState1
     * @param indices1 - in ascending order
     * @param qubitState2
     * @param indices2 - in ascending order
     * @return
     */
    static std::shared_ptr<QubitState>
    combine(const std::shared_ptr<QubitState> &qubitState1, std::vector<int> indices1,
            const std::shared_ptr<QubitState> &qubitState2, std::vector<int> indices2);

    iterator begin() {
        return this->map.begin();
    }

    iterator end() {
        return this->map.end();
    }

    //Define operator []
    Complex &operator[](const BitSet &key) {
        return this->map.operator[](key);
    }

    //Define operator <<
    friend std::ostream &operator<<(std::ostream &os, const QubitState &qubitState) {
        qubitState.print(os);
        return os;
    }

    //Define operator ==
    bool operator==(const QubitState &rhs) const;

    //to_string
    [[nodiscard]] std::string to_string() const;

    /**
     * @return Sum of all probabilities. This should always be 1
     */
    [[nodiscard]] double norm() const;

    void normalize();

    [[nodiscard]] bool canActivate(size_t index) const;

    [[nodiscard]] bool canActivate(const std::vector<size_t> &indices) const;

    [[nodiscard]] bool alwaysActivated(const std::vector<size_t> &indices) const;

    std::pair<size_t, size_t> countActivations(std::vector<size_t> controls);

    void applyGate(size_t target,
                   std::array<Complex, 4> matrix);

    void applyGate(size_t target,
                   const std::vector<size_t> &controls,
                   std::array<Complex, 4> matrix);

private:
    size_t nQubits;
    std::map<BitSet, Complex> map;

    void removeZeroEntries();
};

enum TOP {
    T
};

class QubitStateOrTop {
private:
    std::variant<TOP, std::shared_ptr<QubitState>> variant;
public:
    QubitStateOrTop() : variant(TOP::T) {}

    QubitStateOrTop(TOP top) : variant(top) {}

    QubitStateOrTop(std::shared_ptr<QubitState> qubitState) : variant(qubitState) {}

    QubitStateOrTop(const QubitStateOrTop &qubitStateOrTop) = default;

    QubitStateOrTop &operator=(const QubitStateOrTop &qubitStateOrTop) = default;

    QubitStateOrTop &operator=(const std::shared_ptr<QubitState> &qubitState) {
        this->variant = qubitState;
        return *this;
    }

    QubitStateOrTop &operator=(const TOP &t) {
        this->variant = t;
        return *this;
    }

    bool operator==(const QubitStateOrTop &rhs) const {
        return this->variant == rhs.variant;
    }

    bool operator!=(const QubitStateOrTop &rhs) const {
        return !(rhs == *this);
    }

    ~QubitStateOrTop() = default;

    [[nodiscard]] bool isTop() const {
        return std::holds_alternative<TOP>(variant);
    }

    [[nodiscard]] bool isQubitState() const {
        return std::holds_alternative<std::shared_ptr<QubitState>>(variant);
    }

    [[nodiscard]] std::shared_ptr<QubitState> getQubitState() const {
        return std::get<std::shared_ptr<QubitState>>(variant);
    }

    [[nodiscard]] std::string to_string() const {
        if (isTop()) {
            return "TOP";
        } else {
            return getQubitState()->to_string();
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const QubitStateOrTop &qubitStateOrTop) {
        os << qubitStateOrTop.to_string();
        return os;
    }
};

#endif //QCPROP_QUBITSTATE_HPP
