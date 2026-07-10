#include "ampel_control.h"
#include "ampel_output.h"
#include <string.h>

// static uint8_t zustand = 1;
// static uint8_t letzterZustand;
static uint8_t RequestListe[10] = {0};		// Begrenzt auf da von jedem Button/Sensor nur eine Request gleichzeitig in der Liste sein kann; Kann eig auch nur 5 oder 6 gross sein

static Zustand zustand = ZUSTAND_A;
static Zustand letzterZustand = ZUSTAND_A;
static Zustand next_zustand = ZUSTAND_A;

static Phase phase = PHASE_GREEN;
static uint8_t phase_entry = 1;
static uint32_t phase_start_time = 0;

static const uint8_t state_green[ZUSTAND_ANZAHL][LED_COUNT] =
{

	/* Zustand "0"; Dummy Zeile weil Zustände bei unserer Logik erst bei 1 beginnen */
	{0, 0, 0, 0, 0, 0, 0, 0},

    /* ZUSTAND_A  =1 */
    {1, 1, 1, 0, 0, 0, 0, 0},

    /* ZUSTAND_B =2 */
    {0, 0, 1, 1, 0, 1, 0, 0},

    /* ZUSTAND_C = 3 */
    {0, 1, 0, 0, 1, 1, 0, 0},

    /* ZUSTAND_F1 = 4 */
    {1, 0, 1, 0, 0, 0, 0, 1},

    /* ZUSTAND_F2 = 5 */
    {0, 0, 0, 1, 0, 1, 1, 0}
};

static void publish_green_state(Zustand z);
static void publish_orange_transition(Zustand aktuell, Zustand next);
static void publish_red_transition(Zustand aktuell, Zustand next);
static Zustand calculate_next_state(void);
static int RequestListenCheck(void);
static void remove_first_request(void);

void Ampel_Reset_After_Idle(void)
{
	  for(int i = 0; i<=7; i++)
	  {
		  last_led_state[i] = LED_STATE_UNKNOWN;
		  publish_led(i, "red");
	  }

		  zustand = ZUSTAND_A;
		  letzterZustand = ZUSTAND_A;
		  next_zustand = ZUSTAND_A;


		  for (uint8_t i = 0; i < 9; i++)
		  {
			  RequestListe[i] = 0;
		  }
}

// "Extrahiert" topic und gibt den zugehörigen Zustand zurück den das topic requesten wuerde, wenn es auf true gesetzt ist
uint8_t get_request_from_topic(const char *topic)
{
    if (strcmp(topic, "Input/A/Sensor") == 0)
    {
        return ZUSTAND_A;
    }
    else if (strcmp(topic, "Input/B/Sensor") == 0)
    {
        return ZUSTAND_B;
    }
    else if (strcmp(topic, "Input/C/Sensor") == 0)
    {
        return ZUSTAND_C;
    }
    else if (strcmp(topic, "Input/C/Button") == 0)
    {
        return ZUSTAND_F1;
    }
    else if (strcmp(topic, "Input/A/Button") == 0)
    {
        return ZUSTAND_F2;
    }

    return 0; // Topic ist keine Request
}

// Wenn inhalt des topics = true ist, dann wird Request evtl in die RequestListe aufgenommen (wenn es noch nicht in dieser ist)
uint8_t process_request(uint8_t request)
{
	// Testet ob es eine richtige Request war
    if (request == 0 )
    {
		return 0;
    }

    // Testet ob man sich gerade bereits in dem zustand befindet, den der request anfragt
    if(request == zustand)
    {
        return 0;
    }

    // Test ob Request bereits in der RequestListe vorhanden; Wenn ja dann soll nix passieren
    for (uint8_t i = 0; i < 10; i++)
    {
        if (RequestListe[i] == request)
        {
            return 0; // bereits vorhanden
        }
    }

    // Suche erste freie stelle in der RequestList um Request dort einzufuegen
    for (uint8_t i = 0; i < 10; i++)
    {
        if (RequestListe[i] == 0)
        {
            RequestListe[i] = request;
            return 1; // erfolgreich eingefuegt
        }
    }

    return 0;
}

static void publish_green_state(Zustand z)
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        if (state_green[z][i])
        {
            publish_led(i, "green");
        }
        else
        {
            publish_led(i, "red");
        }
    }
}

// Nur LEDs die im aktuellen zustand grün sind und im nächsten nicht mehr schalten auf orange
// LEDs die im aktuellen und nächsten zustand gruen sind, bleiben grün
static void publish_orange_transition(Zustand aktuell, Zustand next)
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        /*if (state_green[aktuell][i] && !state_green[next][i])
        {
            publish_led(i, "orange");
        }*/
        if (state_green[aktuell][i] && !state_green[next][i])
        {
            if (i == LED_A_FUSS || i == LED_C_FUSS)
                publish_led(i, "red");
            else
                publish_led(i, "orange");
        }
    }
}

// Nur die LEDs auf rot schalten die davor orange waren
static void publish_red_transition(Zustand aktuell, Zustand next)
{
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        if (state_green[aktuell][i] && !state_green[next][i]) {
            publish_led(i, "red");
        }
    }
}

static Zustand calculate_next_state(void)
{
	switch(zustand)
			{
			case ZUSTAND_A: // A
				if((RequestListenCheck() == 0) || (RequestListe[0] == 2))
				{
					return ZUSTAND_B;
					// zustand = ZUSTAND_B; // B
				}
				else if(RequestListe[0] == 3)
				{
					return ZUSTAND_C;
					// zustand = ZUSTAND_C; // C
				}
				else if(RequestListe[0] == 4)
				{
					return ZUSTAND_F1;
					// zustand = ZUSTAND_F1; // F1
				}
				else if(RequestListe[0] == 5)
				{
					return ZUSTAND_F2;
					// zustand = ZUSTAND_F2; // F2
				}
				break;

			case ZUSTAND_B: // B
				if((RequestListenCheck() == 0) || (RequestListe[0] == 3))
				{
					return ZUSTAND_C;
					// zustand = ZUSTAND_C; // C
				}
				else if(RequestListe[0] == 1)
				{
					return ZUSTAND_A;
					// zustand = ZUSTAND_A; // A
				}
				else if(RequestListe[0] == 4)
				{
					return ZUSTAND_F1;
					// zustand = ZUSTAND_F1; // F1
				}
				else if(RequestListe[0] == 5)
				{
					return ZUSTAND_F2;
					// zustand = ZUSTAND_F2; // F2
				}
				break;

			case ZUSTAND_C: // C
				if( ( RequestListenCheck() == 0) || (RequestListe[0] == 1))
				{
					return ZUSTAND_A;
					// zustand = ZUSTAND_A; // A
				}
				else if(RequestListe[0] == 2)
				{
					return ZUSTAND_B;
					// zustand = ZUSTAND_B; // B
				}
				else if(RequestListe[0] == 4)
				{
					return ZUSTAND_F1;
					// zustand = ZUSTAND_F1; // F1
				}
				else if(RequestListe[0] == 5)
				{
					return ZUSTAND_F2;
					// zustand = ZUSTAND_F2; // F2
				}
				break;

			case ZUSTAND_F1: // F1
				if((RequestListe[0] == 1) || (letzterZustand == ZUSTAND_C))
				{
					return ZUSTAND_A;
					// zustand = ZUSTAND_A; // A
				}
				else if((RequestListe[0] == 2) || (letzterZustand == ZUSTAND_A))
				{
					return ZUSTAND_B;
					// zustand = ZUSTAND_B; // B
				}
				else if((RequestListe[0] == 3) || (letzterZustand == ZUSTAND_B))
				{
					return ZUSTAND_C;
					// zustand = ZUSTAND_C; // C
				}
				else if(RequestListe[0] == 5)
				{
					return ZUSTAND_F2;
					// zustand = ZUSTAND_F2; // F2
				}
				break;

			case ZUSTAND_F2: // F2
				if((RequestListe[0] == 1) || (letzterZustand == ZUSTAND_C))
				{
					return ZUSTAND_A;
					// zustand = ZUSTAND_A; // A
				}
				else if((RequestListe[0] == 2) || (letzterZustand == ZUSTAND_A))
				{
					return ZUSTAND_B;
					// zustand = ZUSTAND_B; // B
				}
				else if((RequestListe[0] == 3) || (letzterZustand == ZUSTAND_B))
				{
					return ZUSTAND_C;
					// zustand = ZUSTAND_C; // C
				}
				else if(RequestListe[0] == 4)
				{
					return ZUSTAND_F1;
					// zustand = ZUSTAND_F1; // F1
				}
				break;
			}
	return ZUSTAND_A;
}

// Schaut ob in der RequestListe überhaupt was drinne steht. Wenn nein, dann den automatischen zyklus fortsetzten ( A->B->C )
static int RequestListenCheck(void)
{
	if (RequestListe[0] == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

// Entfernt die aktuellste Request (von einem Button oder Sensor), damit dann die nächste Request nachrücken kann
static void remove_first_request(void)
{
    if (RequestListe[0] == 0)
    {
        return;
    }

    for (uint8_t i = 0; i < 9; i++)
    {
        RequestListe[i] = RequestListe[i + 1];
    }

    RequestListe[9] = 0;
}

void Ampel_zyklus(void)
{
    uint32_t now = HAL_GetTick();

    switch (phase)
    {
        case PHASE_GREEN:
        {
            if (phase_entry)
            {
                phase_entry = 0;
                phase_start_time = now;

                publish_green_state(zustand);
            }

            if (zustand == ZUSTAND_F1 || zustand == ZUSTAND_F2)		// Checken ob man sich in Zustand F1 oder F2 befeindet, den dann wird die orange phase geskippt
            {
				if (now - phase_start_time >= FUSS_GREEN_TIME_MS)	// Fussgänger haben andere zeiten als die normale ampeln
                {
                    next_zustand = calculate_next_state();

                    phase = PHASE_ALL_RED;
                    phase_entry = 1;
                }
            }
            else
            {
                if (now - phase_start_time >= GREEN_TIME_MS)
                {
                    next_zustand = calculate_next_state();

                    phase = PHASE_ORANGE;
                    phase_entry = 1;
                }
            }

            break;
        }

        case PHASE_ORANGE:
        {
            if (phase_entry)
            {
                phase_entry = 0;
                phase_start_time = now;

                publish_orange_transition(zustand, next_zustand);
            }

            if (now - phase_start_time >= ORANGE_TIME_MS)
            {
                phase = PHASE_ALL_RED;
                phase_entry = 1;
            }

            break;
        }

        case PHASE_ALL_RED:
        {
            if (phase_entry)
            {
                phase_entry = 0;
                phase_start_time = now;

                publish_red_transition(zustand, next_zustand);
            }

            if (zustand == ZUSTAND_F1 || zustand == ZUSTAND_F2)		// Wieder wie bei phase grün checken, weil Fussgängerampel andere zyklus dauern hat als die normalen ampeln
            {
                if (now - phase_start_time >= FUSS_RED_TIME_MS)
                {
                    letzterZustand = zustand;
                    zustand = next_zustand;

                    remove_first_request();

                    phase = PHASE_GREEN;
                    phase_entry = 1;
                }
            }
            else
            {
                if (now - phase_start_time >= ALL_RED_TIME_MS)
                {
                    letzterZustand = zustand;
                    zustand = next_zustand;

                    remove_first_request();

                    phase = PHASE_GREEN;
                    phase_entry = 1;
                }
            }

            break;
        }
    }
}
