#ifndef AMPEL_OUTPUT_H
#define AMPEL_OUTPUT_H

#include "master_config.h"

extern LedState last_led_state[LED_COUNT];

void publish_led(uint8_t led, const char *color);
void process_publish_retries(void);

#endif /* AMPEL_OUTPUT_H */
