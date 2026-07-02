#ifndef SENSOR_H
#define SENSOR_H

#define SENSOR_ADC_GRENZWERT      1500
#define SENSOR_DUNKEL_ZEIT_MS     300
#define SENSOR_PUBLISH_COOLDOWN_MS 1000

void Sensor_Update(void);
void Sensor_Process(void);

#endif /* SENSOR_H */
