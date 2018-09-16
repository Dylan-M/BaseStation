/**********************************************************************

Consists.cpp
COPYRIGHT (c) 2018 Dylan J. Myers <ralgith@gmail.com>

Addition to DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************
**********************************************************************/

#include "Consists.h"
#include "SerialCommand.h"
#include "DCCpp_Uno.h"
#include "EEStore.h"
#include <EEPROM.h>
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

Consist *Consist::create(byte address, int leadLoco, int trailLoco, int locos[MAX_CONSIST_SIZE], int v) {
  Consist *consist;
  
  if(firstConsist == NULL) {
    firstConsist = (Consist *)calloc(1,sizeof(Consist));
    consist = firstConsist;
  } else if((consist = get(address)) == NULL) {
    consist = firstConsist;
    while(consist->nextConsist != NULL) {
      consist = consist->nextConsist;
    }
    consist->nextConsist = (Consist *)calloc(1,sizeof(Consist));
    consist = consist->nextConsist;
  }

  if(consist == NULL) {       // problem allocating memory
    if(v == 1) {
      INTERFACE.print("<X>");
    }
    return(consist);
  }
  
  consist->data.address = address;
  consist->data.leadLoco = leadLoco;
  consist->data.trailLoco = trailLoco;
  memcpy(consist->data.locos, locos, sizeof(consist->data.locos));
  if(v == 1) {
    INTERFACE.print("<O>");
  }
  return(consist);
}

///////////////////////////////////////////////////////////////////////////////

void Consist::store() {
  Consist *consist;
  
  consist = firstConsist;
  EEStore::eeStore->data.nConsists = 0;
  
  while(consist != NULL) {
    consist->num=EEStore::pointer();
    EEPROM.put(EEStore::pointer(), consist->data);
    EEStore::advance(sizeof(consist->data));
    consist = consist->nextConsist;
    EEStore::eeStore->data.nConsists++;
  }
}

///////////////////////////////////////////////////////////////////////////////

void Consist::load() {
  struct ConsistData data;
  Consist *consist;

  for(int i = 0; i<EEStore::eeStore->data.nConsists; i++) {
    EEPROM.get(EEStore::pointer(), data);  
    consist = create(data.address, data.leadLoco, data.trailLoco, data.locos);
    consist->num = EEStore::pointer();
    EEStore::advance(sizeof(consist->data));
  }  
}

///////////////////////////////////////////////////////////////////////////////

void Consist::show(int n) {
  Consist *consist;

  if(firstConsist == NULL){
    INTERFACE.print("<X>");
    return;
  }
    
  for(consist = firstConsist; consist != NULL; consist = consist->nextConsist) {
    INTERFACE.print("<H");
    INTERFACE.print(consist->data.address);
    if(n == 1){
      INTERFACE.print(" Lead: ");
      INTERFACE.print(consist->data.leadLoco);
      INTERFACE.print(" Trail: ");
      INTERFACE.print(consist->data.trailLoco);
    }

    boolean other = false;
    for (int i = 0; i < MAX_CONSIST_SIZE; i++) {
      if(consist->data.locos[i] != -1) {
        if (other) {
          INTERFACE.print(", ");
        } else {
          INTERFACE.print(" Others: ");
          other = true;
        }
        INTERFACE.print(consist->data.locos[i]);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

boolean Consist::isInConsist(int loco) {
  Consist *consist;
  for(consist = firstConsist; consist != NULL; consist = consist->nextConsist) {
    if (consist->data.leadLoco == loco || consist->data.trailLoco == loco) {
      return true;
    }
    for (int i = 0; i < MAX_CONSIST_SIZE; i++) {
      if (consist->data.locos[i] == loco) {
        return true;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Consist* Consist::get(int n) {
  Consist *consist;
  for(consist = firstConsist; consist != NULL && consist->data.address != n; consist = consist->nextConsist);
  return(consist); 
}

///////////////////////////////////////////////////////////////////////////////

void Consist::clear(int n) {
  Consist *consist,*pp;
  
  for(consist = firstConsist; consist != NULL && consist->data.address != n; pp = consist, consist=consist->nextConsist);

  if(consist == NULL) {
    INTERFACE.print("<X>");
    return;
  }
  
  if(consist == firstConsist) {
    firstConsist = consist->nextConsist;
  } else {
    pp->nextConsist = consist->nextConsist;
  }

  free(consist);

  INTERFACE.print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

boolean Consist::add(int n, int loco, byte pos = POS_OTHER_LOCO) {
  Consist *consist, *pp;
  boolean assigned = false;

  for(consist = firstConsist; consist != NULL && consist->data.address != n; pp = consist, consist=consist->nextConsist);

  if(consist == NULL || isInConsist(loco)) {
    INTERFACE.print("<X>");
    return assigned;
  }

  if (pos == POS_LEAD_LOCO) {
    if (consist->data.leadLoco != -1) {
      int tmpLoco = consist->data.leadLoco;
      if (!add(n, tmpLoco, POS_OTHER_LOCO)) {
        INTERFACE.print("<X>");
        return assigned;
      }
    }
    consist->data.leadLoco = loco;
    assigned = true;
  } else if (pos == POS_LEAD_LOCO) {
    if (consist->data.trailLoco != -1) {
      int tmpLoco = consist->data.trailLoco;
      if (!add(n, tmpLoco, POS_OTHER_LOCO)) {
        INTERFACE.print("<X>");
        return assigned;
      };
    }
    consist->data.leadLoco = loco;
    assigned = true;
  } else {
    for (int i = 0; i < MAX_CONSIST_SIZE; i++) {
      if(consist->data.locos[i] == -1) {
        consist->data.locos[i] = loco;
        assigned = true;
        break;
      }
    }
  }

  if(assigned) {
    INTERFACE.print("<O>");
  } else {
    INTERFACE.print("<X>");
  }

  return assigned;
}

///////////////////////////////////////////////////////////////////////////////

boolean Consist::remove(int n, int loco) {
//  Consist *consist, *pp;
//  
//  for(consist=firstConsist;consist!=NULL && consist->data.id!=n;pp=consist,consist=consist->nextConsist);
//
//  if(consist==NULL){
//    INTERFACE.print("<X>");
//    return;
//  }
//  
//  if(consist==firstConsist)
//    firstConsist=consist->nextConsist;
//  else
//    pp->nextConsist=consist->nextConsist;
//
//  free(consist);
//
//  INTERFACE.print("<O>");
  return false;
}

///////////////////////////////////////////////////////////////////////////////

void Consist::parse(char *c) {
//  int n,s,m;
//  Consist *consist;
//  
//  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
//    
//    case 2:                     // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
//      consist=get(n);
//      if(consist!=NULL)
//        consist->activate(s);
//      else
//        INTERFACE.print("<X>");
//      break;
//
//    case 3:                     // argument is string with id number of turnout followed by an address and subAddress
//      create(n,s,m,1);
//    break;
//
//    case 1:                     // argument is a string with id number only
//      remove(n);
//    break;
//    
//    case -1:                    // no arguments
//      show(1);                  // verbose show
//    break;
//  }
}

///////////////////////////////////////////////////////////////////////////////

Consist *Consist::firstConsist = NULL;
