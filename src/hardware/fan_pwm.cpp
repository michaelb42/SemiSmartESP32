#include "../../include/hardware/fan_pwm.h"
#include "../../include/core/system_config.h"
#include <avr/io.h>

#if PIN_FAN != 3
#error "fan_pwm uses Timer2 OC2B; PIN_FAN must be 3"
#endif

// Fast PWM TOP=OCR2A -> f ~= F_CPU/(OCR2A+1) @ prescale 1 (~27.5 kHz @ 16 MHz)
#define FAN_TOP 580U

void fanPwmBegin(void) {
    DDRD |= (1 << DDD3);
    PORTD &= ~(1 << PORTD3);
    TCCR2A = (uint8_t)(_BV(WGM21) | _BV(WGM20));
    TCCR2B = (uint8_t)(_BV(WGM22) | _BV(CS20));
    OCR2A = (uint8_t)FAN_TOP;
    OCR2B = 0;
    TCNT2 = 0;
}

void fanPwmWrite(uint8_t d) {
    if (!d) {
        TCCR2A &= (uint8_t)~(_BV(COM2B1) | _BV(COM2B0));
        PORTD &= ~(1 << PORTD3);
        return;
    }
    TCCR2A = (uint8_t)((TCCR2A & (uint8_t)~(_BV(COM2B1) | _BV(COM2B0))) | _BV(COM2B1));
    uint16_t o = (uint16_t)d * FAN_TOP / 255U;
    OCR2B = (uint8_t)((o > FAN_TOP) ? FAN_TOP : (o ? o : 1U));
}
