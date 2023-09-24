#pragma once

#include <chrono>

class Timer {
public:
    Timer() : m_beg(clock_::now()) {
    }
    void reset() {
        m_beg = clock_::now();
    }

    double elapsed() const {
        return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(clock_::now() - m_beg).count());
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1>> second_;
    std::chrono::time_point<clock_> m_beg;
};