#ifndef LM75_H_INCLUDED
#define LM75_H_INCLUDED

//LM75 address (a2 = a1 = a0 = 0)
#define LM75_ADR 0x90

//function headers
double get_temperature(void);
void lm75_init(void);

#endif


