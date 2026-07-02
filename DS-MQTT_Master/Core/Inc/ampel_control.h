#ifndef AMPEL_CONTROL_H
#define AMPEL_CONTROL_H

#include "master_config.h"

void Ampel_Reset_After_Idle(void);
void Ampel_zyklus(void);
uint8_t get_request_from_topic(const char *topic);
uint8_t process_request(uint8_t request);

#endif /* AMPEL_CONTROL_H */
