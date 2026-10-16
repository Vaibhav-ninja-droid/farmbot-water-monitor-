#ifndef IRRIGATION_H
#define IRRIGATION_H

#include <stdbool.h>
#include "tank.h"

bool irrigation_is_allowed(enum tank_state state);

#endif