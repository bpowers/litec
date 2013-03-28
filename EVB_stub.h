#ifndef _EVB_H_
#define _EVB_H_

typedef long int32;
typedef unsigned int uint16;
typedef int int16;
typedef unsigned int uint8;

extern uint16 _H11PORTA;
extern uint16 _H11TMSK2;
extern uint16 _H11TCNT;
extern uint16 _H11OC1M;
extern uint16 _H11OC1D;
extern uint16 _H11TOC1;
extern uint16 _H11TOC2;
extern uint16 _H11TOC3;
extern uint16 _H11TOC4;
extern uint16 _H11TOC5;
extern uint16 _H11TCTL1;
extern uint16 _H11DDRC;
extern uint16 _H11PORTC;
extern uint16 _H11PACTL;
extern uint16 _H11PACNT;
extern uint16 _H11TFLG2;
extern uint16 _H11ADCTL;
extern uint16 _H11ADR1;
extern uint16 _H11ADR2;
extern uint16 _H11ADR3;

extern int16 motorpw;

#define _VECTOR(a, b)
#define __mod2__

#endif /* _EVB_H_ */
