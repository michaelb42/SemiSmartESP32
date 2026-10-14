#ifndef FAN_PWM_H
#define FAN_PWM_H

#include <stdint.h>

/**
 * High-frequency PWM on PIN_FAN (Timer2 / OC2B), ~27.5 kHz — matches Nano R4 master Fan+PwmOut.
 * Requires PIN_FAN == 3 on ATmega328.
 */
void fanPwmBegin(void);
void fanPwmWrite(uint8_t duty255);

#endif
