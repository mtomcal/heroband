/**
 * \file player-vanguard.h
 * \brief Vanguard Heroic Resolve helpers.
 */

#ifndef PLAYER_VANGUARD_H
#define PLAYER_VANGUARD_H

struct player;

enum vanguard_resolve_tier {
	VANGUARD_RESOLVE_STEADY = 0,
	VANGUARD_RESOLVE_TESTED,
	VANGUARD_RESOLVE_RESOLUTE,
	VANGUARD_RESOLVE_UNBROKEN,
	VANGUARD_RESOLVE_LAST_STAND
};

enum vanguard_resolve_damage_source {
	VANGUARD_RESOLVE_DAMAGE_HOSTILE = 0,
	VANGUARD_RESOLVE_DAMAGE_SELF,
	VANGUARD_RESOLVE_DAMAGE_STARVATION,
	VANGUARD_RESOLVE_DAMAGE_SAFE_ATTRITION,
	VANGUARD_RESOLVE_DAMAGE_CORRUPT_OBJECT,
	VANGUARD_RESOLVE_DAMAGE_CURSE,
	VANGUARD_RESOLVE_DAMAGE_NONHOSTILE
};

enum vanguard_resolve_tier vanguard_resolve_pressure_tier(int charges);
enum vanguard_resolve_tier vanguard_resolve_last_stand_tier(int chp, int mhp);
enum vanguard_resolve_tier vanguard_resolve_effective_tier(int charges,
	int chp, int mhp);
const char *vanguard_resolve_tier_name(enum vanguard_resolve_tier tier);
const char *vanguard_resolve_status_name(const struct player *p);
bool vanguard_resolve_damage_source_qualifies(
	enum vanguard_resolve_damage_source source);
int vanguard_resolve_pressure(const struct player *p);
void vanguard_resolve_clear(struct player *p);
bool vanguard_resolve_note_damage(struct player *p,
	enum vanguard_resolve_damage_source source);
bool vanguard_resolve_has_active_combat(const struct player *p);
void vanguard_resolve_tick(struct player *p);
void vanguard_resolve_on_new_level(struct player *p);
void vanguard_resolve_on_rest_full(struct player *p);
int vanguard_resolve_effective_tier_for_player(const struct player *p);
int vanguard_resolve_duration_bonus(const struct player *p);
int vanguard_resolve_melee_blow_bonus(const struct player *p);
int vanguard_resolve_control_bonus(const struct player *p);
int vanguard_resolve_armor_mastery_bonus(const struct player *p,
	int timed_effect);
void vanguard_resolve_save_state(const struct player *p, uint32_t *pressure,
	uint32_t *recent_damage);
void vanguard_resolve_load_state(struct player *p, uint32_t pressure,
	uint32_t recent_damage);
void vanguard_resolve_finalize_load_state(struct player *p);

#endif /* PLAYER_VANGUARD_H */
