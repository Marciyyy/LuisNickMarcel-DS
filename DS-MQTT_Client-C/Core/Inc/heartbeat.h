#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <stdint.h>

#define HEARTBEAT_TIMEOUT_MS 1500

extern uint8_t  idle;
extern uint32_t lastHeartbeatMaster;
extern uint32_t lastHeartbeatA;
extern uint32_t lastHeartbeatB;

void Heartbeat_Send(void);
void Heartbeat_Monitor(void);

#endif /* HEARTBEAT_H */
