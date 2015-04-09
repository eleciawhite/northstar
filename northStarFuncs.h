// northStarFuncs.h 
// these functions control how the northStar code behaves

#ifndef NORTH_STAR_FUNCS_H
#define NORTH_STAR_FUNCS_H

typedef enum {OFF=0, ACCEL=1, MAG=2, GYRO=3} eModes;

uint8_t conditionAccels(float reading);
uint8_t conditionMags(float heading, float offset, float range); 
uint8_t conditionGyros(float reading);
eModes changeMode(eModes current);

#endif /*NORTH_STAR_FUNCS_H*/
