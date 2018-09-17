/**********************************************************************

Consists.h
COPYRIGHT (c) 2018 Dylan J. Myers <ralgith@gmail.com>

Addition to DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Arduino.h"
#include "Config.h"

#ifndef Consists_h
#define Consists_h

#define POS_LEAD_LOCO       0
#define POS_TRAIL_LOCO      1
#define POS_OTHER_LOCO      2
#define MIN_CONSIST_ADDRESS 1
#define MAX_CONSIST_ADDRESS 127
#define CONSIST_NONE        0

struct ConsistData {
  byte address;
  int leadLoco = CONSIST_NONE;
  int trailLoco = CONSIST_NONE;
  // All other locos in the consist, allows for MAX_CONSIST_SIZE+2 (the lead and trail won't be in the array)
  int locos[MAX_CONSIST_SIZE] = { CONSIST_NONE };
};

struct Consist {
  static Consist *firstConsist;
  int num;
  struct ConsistData data;
  Consist *nextConsist;
  static int isInConsist(int);
  static void parse(char *c);
  static Consist* get(int);
  static boolean add(int, int, byte);
  static boolean remove(byte, int);
  static void clear(int);
  static void load();
  static void store();
  static Consist *create(byte, int, int, int[]);
  static void show(int=0);
}; // Consist
  
#endif
