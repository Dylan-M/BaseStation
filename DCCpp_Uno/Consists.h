/**********************************************************************

Consists.h
COPYRIGHT (c) 2018 Dylan J. Myers <ralgith@gmail.com>

Addition to DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Arduino.h"
#include "Config.h"

#ifndef Consists_h
#define Consists_h

#define POS_LEAD_LOCO  0
#define POS_TRAIL_LOCO 1
#define POS_OTHER_LOCO 2

struct ConsistData {
  byte address;
  int leadLoco;
  int trailLoco;
  // All other locos in the consist, allows for MAX_CONSIST_SIZE+2 (the lead and trail won't be in the array)
  int locos[MAX_CONSIST_SIZE] = { -1 };
};

struct Consist {
  static Consist *firstConsist;
  int num;
  struct ConsistData data;
  Consist *nextConsist;
  static void parse(char *c);
  static Consist* get(int);
  static void add(int, byte);
  static void remove(int, int);
  static void clear(int);
  static void load();
  static void store();
  static Consist *create(byte, int, int, int[], int = 0);
  static void show(int=0);
}; // Consist
  
#endif
