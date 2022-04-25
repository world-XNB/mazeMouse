#ifndef _MAIN_H_
#define _MAIN_H_

#include <reg52.h>
#include "intrins.h"
//Infrared sensor
sbit A0=P4^0; 
sbit A1=P2^0; 
sbit A2=P2^7;

sbit IR1=P2^1; 
sbit IR2=P2^2; 
sbit IR3=P2^3; 
sbit IR4=P2^4; 
sbit IR5=P2^5;

//Beep
sbit Beep = P3^7;

//Digital Tube
sbit tube1 = P4^3; 
sbit tube2 = P4^2;

void update();
void note_direction();
unsigned char note_current();
void note_fork();
void beep_on();
#endif 