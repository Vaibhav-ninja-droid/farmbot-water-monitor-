#include "irrigation.h"

bool irrigation_is_allowed(enum tank_state state)
{
    switch (state) {

    case TANK_OK:
    case TANK_FULL:
        return true;

    case TANK_EMPTY:
    case TANK_LOW:
    case TANK_ERROR:
    default:
        return false;
    }
}