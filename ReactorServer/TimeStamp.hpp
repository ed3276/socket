#pragma once

#include <time.h>
#include <stdint.h>
#include <iostream>
#include <string>

class TimeStamp {
 public:
    TimeStamp();
    TimeStamp(int64_t initVal);
    ~TimeStamp();
    time_t ToInt() const;
    std::string ToString() const;

    static TimeStamp Now();
 private:
    time_t t_;
};
