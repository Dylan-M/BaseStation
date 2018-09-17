/**********************************************************************

Consists.cpp
COPYRIGHT (c) 2018 Dylan J. Myers <ralgith@gmail.com>

Addition to DCC++ BASE STATION for the Arduino

**********************************************************************/
/**********************************************************************

This code will allow BASE STATION to track consists of locomotives and
handle them intelligently. It allows for the following behaviors:

1. Only lead or trail locomotives can be used to control the consist
2. Locomotives can only be in a single consist at a time.
3. When using the 'lead' locomotive, the directions are all handled
  the way they were entered into the consist.
4. When using the 'trail' locomotive, the directions are all reversed
  from what was entered into the consist when it was set up.
5. Only the locomotive you've selected (lead or trail) will activate
  function effects such as horn, bell, and ditch lights.
6. New lead or trail locomotives can be set at any time.

Consisting Commands:

<C ADDRESS LEAD_LOCO TRAIL_LOCO OTHER_LOCO1 OTHER_LOCO2 .. OTHER_LOCON>

    Creates a new consist with the consist address set to ADDRESS, the
    consist's lead locomotive set to LEAD_LOCO, the consist's trail
    locomotive set to TRAIL_LOCO, and as many additional locomotives
    (even none) as desired up to MAX_CONSIST_SIZE as defined in
    Config.h.
    Examples:
        <C 39 777 7524> - Valid consist with only lead and trail
        <C 117 472 376 199 9954 1278> - Valid consist with lead, trail
          and three additional locomotives.

<C ADDRESS LOCO>

    Deletes a single specified locomotive (can't have a consist of
    just 1!) from it's consist.

<C ADDRESS>

    Completely clears the entire consist.

<C>

    Show all existing consists.

Where:
    ADDRESS is a valid consist address (1-128 to human, 0-127 to code)
    LEAD_LOCO is the locomotive that is first in the consist
    TRAIL_LOCO is the locomotive that is last in the consist
    OTHER_LOCO1 .. OTHER_LOCON is an unspecified number of additional
      locomotives

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
  
  if (firstConsist == NULL) {
    firstConsist = (Consist *)calloc(1,sizeof(Consist));
    consist = firstConsist;
  } else if ((consist = get(address)) == NULL) {
    consist = firstConsist;
    while (consist->nextConsist != NULL) {
      consist = consist->nextConsist;
    }
    consist->nextConsist = (Consist *)calloc(1,sizeof(Consist));
    consist = consist->nextConsist;
  }

  if(consist == NULL) {       // problem allocating memory
    if (v == 1) {
      INTERFACE.print("<X>");
    }
    return (consist);
  }
  
  consist->data.address = address;
  consist->data.leadLoco = leadLoco;
  consist->data.trailLoco = trailLoco;
  memcpy (consist->data.locos, locos, sizeof(consist->data.locos));
  if(v == 1) {
    INTERFACE.print("<O>");
  }
  return (consist);
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
      if(consist->data.locos[i] != CONSIST_NONE) {
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

int Consist::isInConsist(int loco) {
  Consist *consist;
  for(consist = firstConsist; consist != NULL; consist = consist->nextConsist) {
    if (consist->data.leadLoco == loco || consist->data.trailLoco == loco) {
      return consist->data.address;
    }
    for (int i = 0; i < MAX_CONSIST_SIZE; i++) {
      if (consist->data.locos[i] == loco) {
        return consist->data.address;
      }
    }
  }
  return CONSIST_NONE;
}

///////////////////////////////////////////////////////////////////////////////

Consist* Consist::get(int n) {
  Consist *consist;
  for (consist = firstConsist; consist != NULL && consist->data.address != n; consist = consist->nextConsist);
  return (consist); 
}

///////////////////////////////////////////////////////////////////////////////

void Consist::clear(int n) {
  Consist *consist,*pp;
  
  for (consist = firstConsist; consist != NULL && consist->data.address != n; pp = consist, consist=consist->nextConsist);

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

boolean Consist::add(int addr, int loco, byte pos = POS_OTHER_LOCO) {
  Consist *consist, *pp;
  boolean assigned = false;

  for (consist = firstConsist; consist != NULL && consist->data.address != addr; pp = consist, consist = consist->nextConsist);

  if (consist == NULL || isInConsist(loco) != CONSIST_NONE) {
    INTERFACE.print("<X>");
    return assigned;
  }

  if (pos == POS_LEAD_LOCO) {
    if (consist->data.leadLoco != CONSIST_NONE) {
      int tmpLoco = consist->data.leadLoco;
      if (!add(addr, tmpLoco, POS_OTHER_LOCO)) {
        INTERFACE.print("<X>");
        return assigned;
      }
    }
    consist->data.leadLoco = loco;
    assigned = true;
  } else if (pos == POS_LEAD_LOCO) {
    if (consist->data.trailLoco != CONSIST_NONE) {
      int tmpLoco = consist->data.trailLoco;
      if (!add(addr, tmpLoco, POS_OTHER_LOCO)) {
        INTERFACE.print("<X>");
        return assigned;
      };
    }
    consist->data.leadLoco = loco;
    assigned = true;
  } else {
    for (int i = 0; i < MAX_CONSIST_SIZE; i++) {
      if(consist->data.locos[i] == CONSIST_NONE) {
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

boolean Consist::remove(byte addr, int loco) {
  Consist *consist;
  boolean removed = false;

  if(addr != isInConsist(loco) || isInConsist(loco) == CONSIST_NONE) {
    INTERFACE.print("<X>");
    return removed;
  }

  consist = get(addr);

  if(consist == NULL) {
    INTERFACE.print("<X>");
    return removed;
  }

  if (consist->data.leadLoco == loco) {
    consist->data.leadLoco = CONSIST_NONE;
    removed = true;
  } else if (consist->data.trailLoco == loco) {
    consist->data.trailLoco = CONSIST_NONE;
    removed = true;
  } else {
    for (int i = 0; i < MAX_CONSIST_SIZE; i++) {
      if(consist->data.locos[i] == loco) {
        consist->data.locos[i] = CONSIST_NONE;
        removed = true;
        break;
      }
    }
  }

  if(removed) {
    INTERFACE.print("<O>");
  } else {
    INTERFACE.print("<X>");
  }
  
  return removed;
}

///////////////////////////////////////////////////////////////////////////////

void Consist::parse(char *c) {
  int n,s,m, l;
  int locos[MAX_CONSIST_SIZE] = { -1 };
  Consist *consist;
  
  switch (sscanf(c,"%d %d %d",&n,&s,&m)) {
    case 3:                     // argument is string with consist address, lead, and trail
      {
        int i = 0;
        while (sscanf(c, "%d", &l) > 0) {
          locos[i] = l;
          i++;
        }
        create(n,s,m,locos,1);
      }
      break;
    case 2:                     // argument is string with consist address and locomotive to remove
      remove(n, s);
      break;
    case 1:                     // argument is a string with id number only
      clear(n);
      break;
    case -1:                    // no arguments
      show(1);                  // verbose show
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

Consist *Consist::firstConsist = NULL;
