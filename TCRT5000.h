// TCRT5000.h
#pragma once
#include <Arduino.h>

class TCRT5000 {
    uint8_t pin_;

public:
    explicit TCRT5000(uint8_t pin) : pin_(pin) {
        pinMode(pin_, INPUT);
    }

    bool bright() const {
        return digitalRead(pin_) == HIGH;
    }

    bool dark() const {
        return !bright();
    }
};
