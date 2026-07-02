#ifndef MASTER_CONFIG_H
#define MASTER_CONFIG_H

#include "main.h"

#define HEARTBEAT_TIMEOUT_MS 1500

#define GREEN_TIME_MS       5000
#define ORANGE_TIME_MS      2000
#define ALL_RED_TIME_MS     1000
#define FUSS_GREEN_TIME_MS  5000
#define FUSS_RED_TIME_MS    1000

#define AFTER_IDLE_PAUSE_MS 1000

#define LED_COUNT 8

// Fuer Die publish retry queue:
#define RETRY_QUEUE_SIZE 16
#define RETRY_INTERVAL_MS 20

typedef enum {
    ZUSTAND_A 		= 1,
    ZUSTAND_B 		= 2,
    ZUSTAND_C 		= 3,
    ZUSTAND_F1 		= 4,
    ZUSTAND_F2 		= 5,
    ZUSTAND_ANZAHL 	= 6
} Zustand;

typedef enum {
    PHASE_GREEN 	= 0,
    PHASE_ORANGE 	= 1,
    PHASE_ALL_RED 	= 2
} Phase;

enum {
    LED_A_HAUPT 	= 0,
    LED_A_ABBIEGE 	= 1,
    LED_B_HAUPT 	= 2,
    LED_B_ABBIEGE 	= 3,
    LED_C_HAUPT 	= 4,
    LED_C_ABBIEGE 	= 5,
	LED_A_FUSS 		= 6,
    LED_C_FUSS 		= 7
};

typedef enum {
    LED_STATE_UNKNOWN = 0,
    LED_STATE_RED,
    LED_STATE_ORANGE,
    LED_STATE_GREEN
} LedState;

typedef struct {
    uint8_t led;
    LedState state;
    const char *color;
} RetryItem;

#endif /* MASTER_CONFIG_H */
