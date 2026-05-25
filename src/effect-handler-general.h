/**
 * \file effect-handler-general.h
 * \brief Helpers for General-specific effects.
 */

#ifndef INCLUDED_EFFECT_HANDLER_GENERAL_H
#define INCLUDED_EFFECT_HANDLER_GENERAL_H

#include "z-type.h"

struct monster_race;
struct player;

enum general_formation_effect {
	GENERAL_FORMATION_ARROW_VOLLEY = 1,
	GENERAL_FORMATION_GLORIOUS_CHARGE = 2,
	GENERAL_FORMATION_MARSHALS_BANNER = 3,
};

struct monster_race *general_ally_race(int subtype, int player_level);
void general_banner_clear(struct player *p);
void general_banner_tick(struct player *p);

#endif /* INCLUDED_EFFECT_HANDLER_GENERAL_H */
