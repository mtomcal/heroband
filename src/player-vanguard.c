/**
 * \file player-vanguard.c
 * \brief Vanguard Heroic Resolve helpers.
 */

#include "angband.h"
#include "cave.h"
#include "mon-predicate.h"
#include "monster.h"
#include "object.h"
#include "obj-gear.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-vanguard.h"
#include "project.h"

#define VANGUARD_RESOLVE_PRESSURE_MAX 4
#define VANGUARD_RESOLVE_RECENT_TURNS 5
#define VANGUARD_RESOLVE_DECAY_LIMIT 6
#define VANGUARD_RESOLVE_DECAY_IN_COMBAT 1
#define VANGUARD_RESOLVE_DECAY_OUT_OF_COMBAT 3

static const char *vanguard_resolve_tier_names[] = {
	"Steady",
	"Tested",
	"Resolute",
	"Unbroken",
	"Last Stand",
};

static bool player_is_vanguard(const struct player *p)
{
	return p && p->class && streq(p->class->name, "Vanguard");
}

static void vanguard_resolve_note_changed(struct player *p)
{
	if (!p || !p->upkeep) return;
	p->upkeep->redraw |= PR_STATUS;
	p->upkeep->update |= PU_BONUS;
}

enum vanguard_resolve_tier vanguard_resolve_pressure_tier(int charges)
{
	if (charges >= 4) return VANGUARD_RESOLVE_UNBROKEN;
	if (charges >= 2) return VANGUARD_RESOLVE_RESOLUTE;
	if (charges >= 1) return VANGUARD_RESOLVE_TESTED;
	return VANGUARD_RESOLVE_STEADY;
}

enum vanguard_resolve_tier vanguard_resolve_last_stand_tier(int chp, int mhp)
{
	int pct;

	if (mhp <= 0 || chp <= 0) return VANGUARD_RESOLVE_UNBROKEN;
	pct = (100 * chp) / mhp;

	if (pct <= 10) return VANGUARD_RESOLVE_UNBROKEN;
	if (pct <= 20) return VANGUARD_RESOLVE_RESOLUTE;
	if (pct <= 35) return VANGUARD_RESOLVE_TESTED;
	return VANGUARD_RESOLVE_STEADY;
}

enum vanguard_resolve_tier vanguard_resolve_effective_tier(int charges,
	int chp, int mhp)
{
	enum vanguard_resolve_tier pressure =
		vanguard_resolve_pressure_tier(charges);
	enum vanguard_resolve_tier last_stand =
		vanguard_resolve_last_stand_tier(chp, mhp);
	int effective = (int)pressure + (int)last_stand;

	return MIN(effective, (int)VANGUARD_RESOLVE_LAST_STAND);
}

const char *vanguard_resolve_tier_name(enum vanguard_resolve_tier tier)
{
	if (tier < VANGUARD_RESOLVE_STEADY ||
			tier > VANGUARD_RESOLVE_LAST_STAND) {
		return "Steady";
	}
	return vanguard_resolve_tier_names[tier];
}

const char *vanguard_resolve_status_name(const struct player *p)
{
	static char buf[32];
	enum vanguard_resolve_tier tier;

	if (!player_is_vanguard(p)) return NULL;
	tier = vanguard_resolve_effective_tier(
		p->vanguard_resolve_pressure, p->chp, p->mhp);
	strnfmt(buf, sizeof(buf), "Resolve: %s",
		vanguard_resolve_tier_name(tier));
	return buf;
}

int vanguard_resolve_effective_tier_for_player(const struct player *p)
{
	if (!player_is_vanguard(p)) return 0;
	return (int) vanguard_resolve_effective_tier(
		p->vanguard_resolve_pressure, p->chp, p->mhp);
}

int vanguard_resolve_duration_bonus(const struct player *p)
{
	return 2 * vanguard_resolve_effective_tier_for_player(p);
}

int vanguard_resolve_melee_blow_bonus(const struct player *p)
{
	return vanguard_resolve_effective_tier_for_player(p) / 2;
}

int vanguard_resolve_control_bonus(const struct player *p)
{
	return 2 * vanguard_resolve_effective_tier_for_player(p);
}

static bool vanguard_armor_object_is_clean(const struct object *obj)
{
	return obj && !obj->curses && !object_is_corrupt(obj);
}

int vanguard_resolve_armor_mastery_bonus(const struct player *p,
	int timed_effect)
{
	int i;
	bool has_clean_shield = false;
	bool has_clean_heavy_armor = false;

	if (!player_is_vanguard(p)) return 0;
	if (timed_effect != TMD_SHIELD && timed_effect != TMD_BLESSED &&
			timed_effect != TMD_HERO) {
		return 0;
	}

	for (i = 0; i < p->body.count; i++) {
		struct object *obj = p->body.slots[i].obj;

		if (!vanguard_armor_object_is_clean(obj)) continue;
		if (p->body.slots[i].type == EQUIP_SHIELD &&
				obj->tval == TV_SHIELD) {
			has_clean_shield = true;
		}
		if (p->body.slots[i].type == EQUIP_BODY_ARMOR &&
				(obj->tval == TV_HARD_ARMOR || obj->weight >= 300)) {
			has_clean_heavy_armor = true;
		}
	}

	return (has_clean_shield || has_clean_heavy_armor) ? 2 : 0;
}

bool vanguard_resolve_damage_source_qualifies(
	enum vanguard_resolve_damage_source source)
{
	return source == VANGUARD_RESOLVE_DAMAGE_HOSTILE;
}

int vanguard_resolve_pressure(const struct player *p)
{
	return p ? p->vanguard_resolve_pressure : 0;
}

void vanguard_resolve_clear(struct player *p)
{
	if (!p) return;
	p->vanguard_resolve_pressure = 0;
	p->vanguard_resolve_recent_damage = 0;
	p->vanguard_resolve_decay = 0;
	vanguard_resolve_note_changed(p);
}

bool vanguard_resolve_note_damage(struct player *p,
	enum vanguard_resolve_damage_source source)
{
	if (!player_is_vanguard(p)) return false;
	if (!vanguard_resolve_damage_source_qualifies(source)) return false;

	if (p->vanguard_resolve_pressure < VANGUARD_RESOLVE_PRESSURE_MAX) {
		p->vanguard_resolve_pressure++;
		vanguard_resolve_note_changed(p);
	}
	p->vanguard_resolve_recent_damage = VANGUARD_RESOLVE_RECENT_TURNS;
	return true;
}

bool vanguard_resolve_has_active_combat(const struct player *p)
{
	int i;

	if (!player_is_vanguard(p)) return false;
	if (p->vanguard_resolve_recent_damage > 0) return true;
	if (!cave) return false;

	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		if (!mon->race) continue;
		if (monster_is_visible(mon)) return true;
	}

	return false;
}

static bool vanguard_resolve_has_loaded_hostile_context(const struct player *p)
{
	int i;

	if (!player_is_vanguard(p)) return false;
	if (!cave) return false;

	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		if (!mon->race) continue;
		if (monster_is_visible(mon)) return true;
		if (projectable(cave, p->grid, mon->grid, PROJECT_NONE)) return true;
	}

	return false;
}

void vanguard_resolve_tick(struct player *p)
{
	bool active;

	if (!player_is_vanguard(p)) return;
	active = vanguard_resolve_has_active_combat(p);

	if (p->vanguard_resolve_recent_damage > 0) {
		p->vanguard_resolve_recent_damage--;
	}

	if (p->vanguard_resolve_pressure <= 0) {
		p->vanguard_resolve_decay = 0;
		return;
	}

	p->vanguard_resolve_decay += active ?
		VANGUARD_RESOLVE_DECAY_IN_COMBAT :
		VANGUARD_RESOLVE_DECAY_OUT_OF_COMBAT;
	if (p->vanguard_resolve_decay >= VANGUARD_RESOLVE_DECAY_LIMIT) {
		p->vanguard_resolve_pressure--;
		p->vanguard_resolve_decay = 0;
		vanguard_resolve_note_changed(p);
	}
}

void vanguard_resolve_on_new_level(struct player *p)
{
	vanguard_resolve_clear(p);
}

void vanguard_resolve_on_rest_full(struct player *p)
{
	vanguard_resolve_clear(p);
}

void vanguard_resolve_save_state(const struct player *p, uint32_t *pressure,
	uint32_t *recent_damage)
{
	if (pressure) *pressure = 0;
	if (recent_damage) *recent_damage = 0;
	if (!player_is_vanguard(p) || !vanguard_resolve_has_active_combat(p)) {
		return;
	}

	if (pressure) {
		*pressure = (uint32_t) MIN(p->vanguard_resolve_pressure,
			VANGUARD_RESOLVE_PRESSURE_MAX);
	}
	if (recent_damage) {
		*recent_damage = (uint32_t) MIN(p->vanguard_resolve_recent_damage,
			VANGUARD_RESOLVE_RECENT_TURNS);
	}
}

void vanguard_resolve_load_state(struct player *p, uint32_t pressure,
	uint32_t recent_damage)
{
	if (!p) return;

	p->vanguard_resolve_pressure = 0;
	p->vanguard_resolve_recent_damage = 0;
	p->vanguard_resolve_decay = 0;

	if (!player_is_vanguard(p) || pressure == 0) return;

	p->vanguard_resolve_pressure =
		(int16_t) MIN(pressure, (uint32_t) VANGUARD_RESOLVE_PRESSURE_MAX);
	p->vanguard_resolve_recent_damage =
		(int16_t) MIN(recent_damage,
			(uint32_t) VANGUARD_RESOLVE_RECENT_TURNS);
	vanguard_resolve_note_changed(p);
}

void vanguard_resolve_finalize_load_state(struct player *p)
{
	if (!player_is_vanguard(p)) return;
	if (p->vanguard_resolve_pressure <= 0) return;
	if (p->vanguard_resolve_recent_damage > 0) return;
	if (vanguard_resolve_has_active_combat(p)) return;
	if (vanguard_resolve_has_loaded_hostile_context(p)) return;

	vanguard_resolve_clear(p);
}
