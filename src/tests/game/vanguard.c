/* game/vanguard.c */

#include "unit-test.h"
#include "test-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cave.h"
#include "effects.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "monster.h"
#include "mon-attack.h"
#include "mon-make.h"
#include "mon-util.h"
#include "object.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "player-vanguard.h"
#include "savefile.h"
#include "z-util.h"

struct vanguard_scenario {
	const char *id;
	const char *save_name;
	int level;
	int depth;
	int pressure;
	int hp_percent;
	bool include_monsters;
	bool safe_pressure;
};

static void event_message(game_event_type type, game_event_data *data,
		void *user)
{
	printf("Message: %s\n", data->message.msg);
}

static void println(const char *str)
{
	printf("%s\n", str);
}

int setup_tests(void **state)
{
	plog_aux = println;
	event_add_handler(EVENT_MESSAGE, event_message, NULL);
	event_add_handler(EVENT_INITSTATUS, event_message, NULL);

	set_file_paths();
	init_angband();
#ifdef UNIX
	create_needed_dirs();
#endif

	return 0;
}

int teardown_tests(void *state)
{
	file_delete("VanguardResolveCombatTest");
	file_delete("VanguardResolveSafeTest");
	file_delete("VanguardResolveVisibleTest");
	file_delete("VanguardScenarioTest");
	if (cave) wipe_mon_list(cave, player);
	cleanup_angband();
	return 0;
}

static void reset_before_load(void)
{
	play_again = true;
	if (cave) wipe_mon_list(cave, player);
	cleanup_angband();
	chunk_list_max = 0;
	init_angband();
	play_again = false;
}

static bool vanguard_scenario_only(void)
{
	const char *value = getenv("HEROBAND_VANGUARD_SCENARIO_ONLY");

	return value && value[0];
}

static int find_spell(const char *name)
{
	int i;

	for (i = 0; i < player->class->magic.total_spells; i++) {
		const struct class_spell *spell = spell_by_index(player, i);
		if (streq(spell->name, name)) return i;
	}

	return -1;
}

static const struct vanguard_scenario *vanguard_scenario_by_id(
		const char *id)
{
	static const struct vanguard_scenario scenarios[] = {
		{ "vanguard-l01-birth-smoke", "vanguard-l01-birth-smoke",
			1, 1, 0, 100, false, false },
		{ "vanguard-l20-enemy-pressure",
			"vanguard-l20-enemy-pressure", 20, 20, 2, 80, true,
			false },
		{ "vanguard-l20-abuse-rejection",
			"vanguard-l20-abuse-rejection", 20, 20, 0, 100, false,
			false },
		{ "vanguard-l40-last-stand", "vanguard-l40-last-stand",
			40, 40, 2, 19, true, false },
		{ "vanguard-l35-resolve-save-load",
			"vanguard-l35-resolve-save-load", 35, 35, 3, 34, true,
			false },
		{ "vanguard-l35-resolve-safe-load",
			"vanguard-l35-resolve-safe-load", 35, 35, 4, 34, false,
			true },
		{ NULL, NULL, 0, 0, 0, 0, false, false }
	};
	int i;

	if (!id || !id[0]) {
		id = "vanguard-l01-birth-smoke";
	}
	for (i = 0; scenarios[i].id; ++i) {
		if (streq(scenarios[i].id, id)) {
			return &scenarios[i];
		}
	}
	return NULL;
}

static void learn_available_vanguard_orders(void)
{
	int i;

	player->upkeep->new_spells = 0;
	for (i = 0; i < player->class->magic.total_spells; i++) {
		const struct class_spell *spell = spell_by_index(player, i);

		if (spell->slevel <= player->lev) {
			player->spell_flags[i] = PY_SPELL_LEARNED;
			player->spell_order[i] = i;
		} else {
			player->spell_order[i] = 99;
		}
	}
}

static bool add_vanguard_book(int bidx)
{
	const struct class_book *book = &player->class->magic.books[bidx];
	struct object_kind *kind = lookup_kind(book->tval, book->sval);
	struct object *obj;

	if (!kind) return false;
	obj = object_new();
	object_prep(obj, kind, 0, RANDOMISE);
	obj->number = 1;
	obj->known = object_new();
	object_set_base_known(player, obj);
	object_touch(player, obj);
	gear_insert_end(player, obj);
	player->upkeep->total_weight += object_weight_one(obj);
	return object_is_carried(player, obj);
}

static bool place_required_named_monster_near(const char *race_name,
		int preferred_dx, int preferred_dy)
{
	static const int offsets[][2] = {
		{ 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
		{ 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 },
		{ 2, 0 }, { -2, 0 }, { 0, 2 }, { 0, -2 }
	};
	struct monster_race *race = lookup_monster(race_name);
	struct monster_group_info info = { 0, 0 };
	size_t i;

	if (!race) return false;
	for (i = 0; i < N_ELEMENTS(offsets); ++i) {
		struct loc grid = loc(player->grid.x + preferred_dx + offsets[i][0],
			player->grid.y + preferred_dy + offsets[i][1]);

		if (!square_in_bounds(cave, grid)) continue;
		if (square_monster(cave, grid)) delete_monster(cave, grid);
		square_set_feat(cave, grid, FEAT_FLOOR);
		if (!square_isempty(cave, grid)) continue;
		if (place_new_monster(cave, grid, race, false, false, info,
				ORIGIN_DROP)) {
			struct monster *mon = square_monster(cave, grid);

			if (mon) {
				mflag_on(mon->mflag, MFLAG_VISIBLE);
				mon->m_timed[MON_TMD_SLEEP] = 0;
			}
			return true;
		}
	}
	return false;
}

static void prepare_vanguard_scenario_staging_area(void)
{
	struct loc staging = loc(cave->width / 2, cave->height / 2);
	int dy;
	int dx;

	if (square_in_bounds(cave, staging) && square_monster(cave, staging)) {
		delete_monster(cave, staging);
	}
	if (square_in_bounds(cave, staging)) {
		square_set_feat(cave, staging, FEAT_FLOOR);
		player_place(cave, player, staging);
	}
	for (dy = -8; dy <= 8; ++dy) {
		for (dx = -8; dx <= 8; ++dx) {
			struct loc grid = loc(player->grid.x + dx,
				player->grid.y + dy);

			if (!square_in_bounds(cave, grid)) continue;
			if (square_monster(cave, grid)) delete_monster(cave, grid);
			square_set_feat(cave, grid, FEAT_FLOOR);
		}
	}
}

static bool add_vanguard_scenario_monsters(
		const struct vanguard_scenario *scenario)
{
	if (!scenario->include_monsters) return true;
	if (!place_required_named_monster_near("kobold", 1, 0)) return false;
	if (!place_required_named_monster_near("orc shaman", 3, 0)) return false;
	if (scenario->level >= 35 &&
			!place_required_named_monster_near("uruk", 2, 1)) {
		return false;
	}
	return true;
}

static void clear_vanguard_scenario_monsters(void)
{
	int i;

	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);

		if (mon->race) delete_monster(cave, mon->grid);
	}
}


static bool prepare_vanguard_scenario(
		const struct vanguard_scenario *scenario)
{
	const char *race = getenv("HEROBAND_VANGUARD_RACE");
	const char *class_name = getenv("HEROBAND_VANGUARD_CLASS");
	const char *level_text = getenv("HEROBAND_VANGUARD_LEVEL");
	const char *depth_text = getenv("HEROBAND_VANGUARD_DEPTH");
	int level = level_text && level_text[0] ? atoi(level_text) :
		scenario->level;
	int depth = depth_text && depth_text[0] ? atoi(depth_text) :
		scenario->depth;
	int i;

	if (!race || !race[0]) race = "Human";
	if (!class_name || !class_name[0]) class_name = "Vanguard";
	level = MIN(MAX(level, 1), PY_MAX_LEVEL);
	depth = MAX(depth, 0);

	if (!player_make_simple(race, class_name, "Tester")) return false;
	player->depth = depth;
	prepare_next_level(player);
	on_new_level();
	prepare_vanguard_scenario_staging_area();
	clear_vanguard_scenario_monsters();

	player->lev = level;
	player->max_lev = level;
	player->exp = player_exp[level - 1];
	player->max_exp = player->exp;
	player->msp = MAX(player->msp, 40 + level * 4);
	player->csp = player->msp;
	player->au = 1000;
	player->upkeep->update |= PU_BONUS | PU_HP | PU_SPELLS;
	update_stuff(player);
	player->chp = MAX(1, (player->mhp * scenario->hp_percent) / 100);
	player->chp_frac = 0;

	if (player->class->magic.num_books > 1 && level >= 15 &&
			!add_vanguard_book(1)) {
		return false;
	}
	if (player->class->magic.num_books > 2 && level >= 30 &&
			!add_vanguard_book(2)) {
		return false;
	}
	learn_available_vanguard_orders();
	if (!add_vanguard_scenario_monsters(scenario)) return false;

	vanguard_resolve_clear(player);
	if (scenario->safe_pressure) {
		player->vanguard_resolve_pressure = scenario->pressure;
		player->vanguard_resolve_recent_damage = 0;
		player->vanguard_resolve_decay = 0;
	} else {
		for (i = 0; i < scenario->pressure; i++) {
			if (!vanguard_resolve_note_damage(player,
					VANGUARD_RESOLVE_DAMAGE_HOSTILE)) {
				return false;
			}
		}
	}
	player->upkeep->update |= PU_BONUS | PU_TORCH;
	update_stuff(player);
	return true;
}

static bool prepare_high_level_vanguard(void)
{
	int i;

	if (!player_make_simple("Human", "Vanguard", "Tester")) return false;
	prepare_next_level(player);
	on_new_level();
	player->lev = 50;
	player->max_lev = 50;
	player->exp = 99999999;
	player->max_exp = player->exp;
	player->mhp = 500;
	player->chp = player->mhp;
	player->msp = 500;
	player->csp = player->msp;
	player->upkeep->new_spells = player->class->magic.total_spells;

	for (i = 0; i < player->class->magic.total_spells; i++) {
		player->spell_flags[i] = PY_SPELL_LEARNED;
		player->spell_order[i] = i;
		player->upkeep->new_spells--;
	}

	return true;
}

static bool cast_until_active(const char *name, int timed_effect)
{
	int spell = find_spell(name);
	int i;

	require(spell >= 0);
	player->timed[timed_effect] = 0;

	for (i = 0; i < 20; i++) {
		player->msp = 500;
		player->csp = player->msp;
		require(spell_cast(spell, DIR_NONE, NULL));
		if (player->timed[timed_effect] > 0) return true;
	}

	return false;
}

static bool effect_chain_timed_inc(const struct effect *effect, int timed)
{
	const struct effect *e;

	for (e = effect; e; e = e->next) {
		if (e->index != EF_TIMED_INC) {
			continue;
		}
		if (e->subtype == timed) {
			return true;
		}
	}

	return false;
}

static int assert_order_list_is_clean(void)
{
	static const char *expected[] = {
		"Assess the Field",
		"Stand Firm",
		"Whirlwind Attack",
		"Shatter Stone",
		"Breakthrough",
		"Combat Discipline",
		"Staggering Blow",
		"Horn of Defiance",
		"Defend the Weak",
		"Unbroken",
		"Last Stand",
		"Forceful Blow",
		"Brace for Impact",
	};
	static const char *forbidden[] = {
		"Blood", "blood", "Shadow", "shadow", "Nether", "nether",
		"Curse", "curse", "Demon", "demon", "Soul", "soul",
		"Unholy", "unholy", "Werewolf", "werewolf", "Torment",
		"torment", "Blight", "blight", "Corruption", "corruption",
	};
	int i, j;

	eq(player->class->magic.total_spells, (int) N_ELEMENTS(expected));

	for (i = 0; i < player->class->magic.total_spells; i++) {
		const struct class_spell *spell = spell_by_index(player, i);
		eq(streq(spell->name, expected[i]), true);
		eq(streq(spell->realm->name, "tactics"), true);
		for (j = 0; j < (int) N_ELEMENTS(forbidden); j++) {
			require(strstr(spell->name, forbidden[j]) == NULL);
			if (spell->text) {
				require(strstr(spell->text, forbidden[j]) == NULL);
			}
		}
		require(!effect_chain_timed_inc(spell->effect, TMD_BLOODLUST));
		require(!effect_chain_timed_inc(spell->effect, TMD_ATT_VAMP));
	}

	return 0;
}

static int assert_resolve_orders_activate_clean_buffs(void)
{
	require(cast_until_active("Stand Firm", TMD_BLESSED));
	require(player->timed[TMD_HERO] > 0);
	eq(player->timed[TMD_SHIELD], 0);

	require(cast_until_active("Combat Discipline", TMD_OPP_CONF));
	require(player->timed[TMD_FREE_ACT] > 0);

	require(cast_until_active("Unbroken", TMD_FAST));
	require(player->timed[TMD_HERO] > 0);
	require(player->timed[TMD_SHIELD] > 0);

	require(cast_until_active("Last Stand", TMD_BLESSED));
	require(player->timed[TMD_HERO] > 0);
	require(player->timed[TMD_SHIELD] > 0);

	require(cast_until_active("Brace for Impact", TMD_BLESSED));
	require(player->timed[TMD_SHIELD] > 0);

	return 0;
}

static int assert_frontline_orders_affect_adjacent_enemy(void)
{
	struct monster *mon;
	int old_hp;

	cave = t_build_arena(20, 20);
	player->grid = loc(10, 10);
	mon = t_add_monster(cave, loc(11, 10), "kobold");
	old_hp = mon->hp;

	player->csp = player->msp;
	require(spell_cast(find_spell("Whirlwind Attack"), DIR_NONE, NULL));
	require(mon->hp < old_hp || mon->midx == 0);

	mon = square_monster(cave, loc(11, 10));
	if (!mon) {
		mon = t_add_monster(cave, loc(11, 10), "kobold");
	}
	old_hp = mon->hp;
	player->csp = player->msp;
	require(spell_cast(find_spell("Forceful Blow"), DIR_E, NULL));
	require(mon->hp < old_hp || mon->midx == 0);
	return 0;
}

static int assert_hostile_monster_damage_builds_pressure(void)
{
	struct monster *mon;
	int old_hp;
	int i;

	cave = t_build_arena(20, 20);
	player->grid = loc(10, 10);
	mon = t_add_monster(cave, loc(11, 10), "kobold");
	mflag_on(mon->mflag, MFLAG_VISIBLE);
	player->mhp = 500;
	player->chp = player->mhp;
	player->state.ac = 0;
	player->state.to_a = 0;
	vanguard_resolve_clear(player);
	require(vanguard_resolve_has_active_combat(player));

	for (i = 0; i < 20 && vanguard_resolve_pressure(player) == 0; i++) {
		old_hp = player->chp;
		(void) make_attack_normal(mon, player);
		if (player->chp < old_hp) break;
	}

	require(player->chp < player->mhp);
	require(vanguard_resolve_pressure(player) > 0);
	return 0;
}

static void build_full_pressure(void)
{
	int i;

	vanguard_resolve_clear(player);
	for (i = 0; i < 4; i++) {
		(void) vanguard_resolve_note_damage(player,
			VANGUARD_RESOLVE_DAMAGE_HOSTILE);
	}
}

static int assert_resolve_passive_and_first_orders_scale(void)
{
	struct player_state base_state;
	struct player_state high_state;
	int spell;
	int base_guard;
	int high_guard;
	int base_blessed;
	int high_blessed;

	vanguard_resolve_clear(player);
	calc_bonuses(player, &base_state, false, true);
	build_full_pressure();
	calc_bonuses(player, &high_state, false, true);
	require(high_state.to_a > base_state.to_a);
	require(high_state.to_h > base_state.to_h);

	rand_fix(50);
	player->chp = player->mhp;
	vanguard_resolve_clear(player);
	player->timed[TMD_BLESSED] = 0;
	player->timed[TMD_HERO] = 0;
	player->timed[TMD_SHIELD] = 0;
	require(cast_until_active("Stand Firm", TMD_BLESSED));
	base_guard = player->timed[TMD_BLESSED];
	eq(player->timed[TMD_SHIELD], 0);
	build_full_pressure();
	player->timed[TMD_BLESSED] = 0;
	player->timed[TMD_HERO] = 0;
	require(cast_until_active("Stand Firm", TMD_BLESSED));
	high_guard = player->timed[TMD_BLESSED];
	eq(player->timed[TMD_SHIELD], 0);
	require(high_guard > base_guard);
	eq(vanguard_resolve_pressure(player), 4);

	player->chp = player->mhp;
	vanguard_resolve_clear(player);
	require(cast_until_active("Last Stand", TMD_BLESSED));
	base_blessed = player->timed[TMD_BLESSED];
	build_full_pressure();
	player->chp = player->mhp / 10;
	player->timed[TMD_BLESSED] = 0;
	player->timed[TMD_HERO] = 0;
	player->timed[TMD_SHIELD] = 0;
	require(cast_until_active("Last Stand", TMD_BLESSED));
	high_blessed = player->timed[TMD_BLESSED];
	require(high_blessed > base_blessed);
	eq(vanguard_resolve_pressure(player), 4);

	spell = find_spell("Staggering Blow");
	require(spell >= 0);
	require(vanguard_resolve_melee_blow_bonus(player) > 0);

	return 0;
}

static int assert_remaining_orders_scale_and_utilities_do_not(void)
{
	int base_fast;
	int high_fast;
	int base_taunt;
	int high_taunt;
	int base_blessed;
	int high_blessed;
	int before_pressure;

	rand_fix(50);

	player->chp = player->mhp;
	vanguard_resolve_clear(player);
	require(cast_until_active("Unbroken", TMD_FAST));
	base_fast = player->timed[TMD_FAST];
	build_full_pressure();
	player->timed[TMD_FAST] = 0;
	player->timed[TMD_HERO] = 0;
	player->timed[TMD_SHIELD] = 0;
	require(cast_until_active("Unbroken", TMD_FAST));
	high_fast = player->timed[TMD_FAST];
	require(high_fast > base_fast);

	vanguard_resolve_clear(player);
	require(cast_until_active("Defend the Weak", TMD_TAUNT));
	base_taunt = player->timed[TMD_TAUNT];
	build_full_pressure();
	player->timed[TMD_TAUNT] = 0;
	require(cast_until_active("Defend the Weak", TMD_TAUNT));
	high_taunt = player->timed[TMD_TAUNT];
	require(high_taunt > base_taunt);

	vanguard_resolve_clear(player);
	require(cast_until_active("Brace for Impact", TMD_BLESSED));
	base_blessed = player->timed[TMD_BLESSED];
	build_full_pressure();
	player->timed[TMD_BLESSED] = 0;
	player->timed[TMD_SHIELD] = 0;
	require(cast_until_active("Brace for Impact", TMD_BLESSED));
	high_blessed = player->timed[TMD_BLESSED];
	require(high_blessed > base_blessed);

	require(vanguard_resolve_melee_blow_bonus(player) > 0);
	require(vanguard_resolve_control_bonus(player) > 0);

	before_pressure = vanguard_resolve_pressure(player);
	require(spell_cast(find_spell("Assess the Field"), DIR_NONE, NULL));
	eq(vanguard_resolve_pressure(player), before_pressure);

	return 0;
}

static int assert_vanguard_resolve_savefile_round_trip(void)
{
	struct monster *mon;
	int i;

	file_delete("VanguardResolveCombatTest");
	file_delete("VanguardResolveSafeTest");
	file_delete("VanguardResolveVisibleTest");

	vanguard_resolve_clear(player);
	require(vanguard_resolve_note_damage(player,
		VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	require(vanguard_resolve_note_damage(player,
		VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	eq(vanguard_resolve_pressure(player), 2);
	require(vanguard_resolve_has_active_combat(player));
	require(savefile_save("VanguardResolveCombatTest"));

	vanguard_resolve_clear(player);
	cave = t_build_arena(20, 20);
	player->grid = loc(10, 10);
	for (i = 0; i < 4; i++) {
		require(vanguard_resolve_note_damage(player,
			VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	}
	eq(vanguard_resolve_pressure(player), 4);
	for (i = 0; i < 5; i++) {
		vanguard_resolve_tick(player);
	}
	require(!vanguard_resolve_has_active_combat(player));
	require(savefile_save("VanguardResolveSafeTest"));

	vanguard_resolve_clear(player);
	cave = t_build_arena(20, 20);
	player->grid = loc(10, 10);
	mon = t_add_monster(cave, loc(11, 10), "kobold");
	require(mon != NULL);
	mflag_on(mon->mflag, MFLAG_VISIBLE);
	player->vanguard_resolve_pressure = 3;
	player->vanguard_resolve_recent_damage = 0;
	player->vanguard_resolve_decay = 0;
	require(vanguard_resolve_has_active_combat(player));
	require(savefile_save("VanguardResolveVisibleTest"));

	reset_before_load();
	require(savefile_load("VanguardResolveCombatTest", false));
	on_new_level();
	eq(vanguard_resolve_pressure(player), 2);
	require(vanguard_resolve_has_active_combat(player));

	reset_before_load();
	require(savefile_load("VanguardResolveSafeTest", false));
	on_new_level();
	eq(vanguard_resolve_pressure(player), 0);
	require(!vanguard_resolve_has_active_combat(player));

	reset_before_load();
	require(savefile_load("VanguardResolveVisibleTest", false));
	on_new_level();
	eq(vanguard_resolve_pressure(player), 3);
	eq(player->vanguard_resolve_recent_damage, 0);

	return 0;
}

static void write_vanguard_scenario_report(const char *path,
		const struct vanguard_scenario *scenario)
{
	ang_file *file;
	int i;

	if (!path || !path[0]) return;
	file = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (!file) return;

	file_putf(file, "## Exact Generated State\n\n");
	file_putf(file, "- Scenario ID: %s\n", scenario->id);
	file_putf(file, "- Name: %s\n", player->full_name);
	file_putf(file, "- Race: %s\n", player->race->name);
	file_putf(file, "- Class: %s\n", player->class->name);
	file_putf(file, "- Level: %d\n", player->lev);
	file_putf(file, "- Experience: %ld\n", (long) player->exp);
	file_putf(file, "- HP: %d/%d\n", player->chp, player->mhp);
	file_putf(file, "- SP: %d/%d\n", player->csp, player->msp);
	file_putf(file, "- Gold: %ld\n", (long) player->au);
	file_putf(file, "- Depth: %d\n", player->depth);
	file_putf(file, "- Starting grid: y=%d x=%d\n", player->grid.y,
		player->grid.x);
	file_putf(file, "- Resolve pressure before save: %d\n",
		player->vanguard_resolve_pressure);
	file_putf(file, "- Resolve recent hostile damage timer before save: %d\n",
		player->vanguard_resolve_recent_damage);
	file_putf(file, "- Resolve active combat before save: %s\n",
		vanguard_resolve_has_active_combat(player) ? "yes" : "no");
	file_putf(file, "- Resolve visible status before save: %s\n",
		vanguard_resolve_status_name(player));
	file_putf(file, "- Safe-context pressure variant: %s\n",
		scenario->safe_pressure ? "yes" : "no");
	file_putf(file, "- Expected loaded pressure: %d\n",
		(scenario->safe_pressure || !vanguard_resolve_has_active_combat(player)) ?
		0 : scenario->pressure);
	file_putf(file, "- Equipment and inventory:\n");
	for (struct object *obj = player->gear; obj; obj = obj->next) {
		char o_name[120];

		object_desc(o_name, sizeof(o_name), obj,
			ODESC_PREFIX | ODESC_FULL, player);
		file_putf(file, "  - %c) %s equipped=%s carried=%s\n",
			gear_to_label(player, obj), o_name,
			object_is_equipped(player->body, obj) ? "yes" : "no",
			object_is_carried(player, obj) ? "yes" : "no");
	}
	file_putf(file, "- Learned orders:\n");
	for (i = 0; i < player->class->magic.total_spells; i++) {
		if (player->spell_flags[i] & PY_SPELL_LEARNED) {
			const struct class_spell *spell = spell_by_index(player, i);

			file_putf(file, "  - %s\n", spell->name);
		}
	}
	file_putf(file, "- Nearby monsters:\n");
	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);

		if (mon->race) {
			file_putf(file, "  - %s at y=%d x=%d awake=%s visible=%s\n",
				mon->race->name, mon->grid.y, mon->grid.x,
				mon->m_timed[MON_TMD_SLEEP] ? "no" : "yes",
				monster_is_visible(mon) ? "yes" : "no");
		}
	}
	file_close(file);
}

static int test_armor_mastery_requires_clean_shield_or_heavy_armor(void *state)
{
	struct player_class vanguard = { .name = "Vanguard" };
	struct equip_slot slots[2] = {
		{ .type = EQUIP_SHIELD, .name = "arm" },
		{ .type = EQUIP_BODY_ARMOR, .name = "body" },
	};
	struct player p = {
		.class = &vanguard,
		.body = { .count = 2, .slots = slots },
	};
	struct object shield = { .tval = TV_SHIELD, .weight = 60 };
	struct object heavy = { .tval = TV_HARD_ARMOR, .weight = 300 };
	struct object light = { .tval = TV_SOFT_ARMOR, .weight = 50 };
	struct curse_data curses[1] = { { .power = 1 } };
	struct ego_item corrupt_ego = { .corrupt = true };

	if (vanguard_scenario_only()) ok;

	slots[0].obj = NULL;
	slots[1].obj = NULL;
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_SHIELD), 0);

	slots[0].obj = &shield;
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_SHIELD), 2);
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_TAUNT), 0);

	slots[0].obj = NULL;
	slots[1].obj = &heavy;
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_BLESSED), 2);

	slots[1].obj = &light;
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_SHIELD), 0);

	shield.curses = curses;
	slots[0].obj = &shield;
	slots[1].obj = NULL;
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_SHIELD), 0);

	shield.curses = NULL;
	shield.ego = &corrupt_ego;
	eq(vanguard_resolve_armor_mastery_bonus(&p, TMD_SHIELD), 0);

	ok;
}

static int test_heroic_resolve_tiers(void *state)
{
	if (vanguard_scenario_only()) ok;

	eq(vanguard_resolve_pressure_tier(0), VANGUARD_RESOLVE_STEADY);
	eq(vanguard_resolve_pressure_tier(1), VANGUARD_RESOLVE_TESTED);
	eq(vanguard_resolve_pressure_tier(2), VANGUARD_RESOLVE_RESOLUTE);
	eq(vanguard_resolve_pressure_tier(4), VANGUARD_RESOLVE_UNBROKEN);

	eq(vanguard_resolve_last_stand_tier(100, 100),
		VANGUARD_RESOLVE_STEADY);
	eq(vanguard_resolve_last_stand_tier(34, 100),
		VANGUARD_RESOLVE_TESTED);
	eq(vanguard_resolve_last_stand_tier(19, 100),
		VANGUARD_RESOLVE_RESOLUTE);
	eq(vanguard_resolve_last_stand_tier(9, 100),
		VANGUARD_RESOLVE_UNBROKEN);

	eq(vanguard_resolve_effective_tier(0, 100, 100),
		VANGUARD_RESOLVE_STEADY);
	eq(vanguard_resolve_effective_tier(2, 100, 100),
		VANGUARD_RESOLVE_RESOLUTE);
	eq(vanguard_resolve_effective_tier(2, 19, 100),
		VANGUARD_RESOLVE_LAST_STAND);
	eq(vanguard_resolve_effective_tier(4, 34, 100),
		VANGUARD_RESOLVE_LAST_STAND);

	eq(streq(vanguard_resolve_tier_name(VANGUARD_RESOLVE_STEADY),
		"Steady"), true);
	eq(streq(vanguard_resolve_tier_name(VANGUARD_RESOLVE_TESTED),
		"Tested"), true);
	eq(streq(vanguard_resolve_tier_name(VANGUARD_RESOLVE_RESOLUTE),
		"Resolute"), true);
	eq(streq(vanguard_resolve_tier_name(VANGUARD_RESOLVE_UNBROKEN),
		"Unbroken"), true);
	eq(streq(vanguard_resolve_tier_name(VANGUARD_RESOLVE_LAST_STAND),
		"Last Stand"), true);

	ok;
}

static int test_heroic_resolve_pressure_sources(void *state)
{
	struct player_class vanguard = { .name = "Vanguard" };
	struct player p = { .class = &vanguard };
	int i;

	if (vanguard_scenario_only()) ok;

	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_HOSTILE), true);
	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_SELF), false);
	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_STARVATION), false);
	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_SAFE_ATTRITION), false);
	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_CORRUPT_OBJECT), false);
	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_CURSE), false);
	eq(vanguard_resolve_damage_source_qualifies(
		VANGUARD_RESOLVE_DAMAGE_NONHOSTILE), false);

	eq(vanguard_resolve_pressure(&p), 0);
	require(vanguard_resolve_note_damage(&p,
		VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	eq(vanguard_resolve_pressure(&p), 1);

	for (i = 0; i < 10; i++) {
		require(vanguard_resolve_note_damage(&p,
			VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	}
	eq(vanguard_resolve_pressure(&p), 4);

	vanguard_resolve_clear(&p);
	eq(vanguard_resolve_pressure(&p), 0);

	for (i = VANGUARD_RESOLVE_DAMAGE_SELF;
			i <= VANGUARD_RESOLVE_DAMAGE_NONHOSTILE; i++) {
		require(!vanguard_resolve_note_damage(&p, i));
		eq(vanguard_resolve_pressure(&p), 0);
	}

	ok;
}

static int test_heroic_resolve_decay_and_cleanup(void *state)
{
	struct player_class vanguard = { .name = "Vanguard" };
	struct player p = { .class = &vanguard, .mhp = 100, .chp = 19 };
	int i;

	if (vanguard_scenario_only()) ok;

	for (i = 0; i < 4; i++) {
		require(vanguard_resolve_note_damage(&p,
			VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	}
	eq(vanguard_resolve_pressure(&p), 4);
	eq(vanguard_resolve_has_active_combat(&p), true);
	eq(vanguard_resolve_effective_tier(vanguard_resolve_pressure(&p),
		p.chp, p.mhp), VANGUARD_RESOLVE_LAST_STAND);

	p.chp = 40;
	eq(vanguard_resolve_pressure(&p), 4);
	eq(vanguard_resolve_effective_tier(vanguard_resolve_pressure(&p),
		p.chp, p.mhp), VANGUARD_RESOLVE_UNBROKEN);

	for (i = 0; i < 5; i++) {
		vanguard_resolve_tick(&p);
	}
	eq(vanguard_resolve_pressure(&p), 4);

	vanguard_resolve_tick(&p);
	eq(vanguard_resolve_pressure(&p), 3);
	vanguard_resolve_tick(&p);
	eq(vanguard_resolve_pressure(&p), 3);
	vanguard_resolve_tick(&p);
	eq(vanguard_resolve_pressure(&p), 2);

	vanguard_resolve_on_rest_full(&p);
	eq(vanguard_resolve_pressure(&p), 0);

	for (i = 0; i < 3; i++) {
		require(vanguard_resolve_note_damage(&p,
			VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	}
	vanguard_resolve_on_new_level(&p);
	eq(vanguard_resolve_pressure(&p), 0);

	ok;
}

static int test_heroic_resolve_status_names(void *state)
{
	struct player_class vanguard = { .name = "Vanguard" };
	struct player p = { .class = &vanguard, .mhp = 100, .chp = 100 };

	if (vanguard_scenario_only()) ok;

	eq(streq(vanguard_resolve_status_name(&p), "Resolve: Steady"), true);
	require(vanguard_resolve_note_damage(&p,
		VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	eq(streq(vanguard_resolve_status_name(&p), "Resolve: Tested"), true);
	require(vanguard_resolve_note_damage(&p,
		VANGUARD_RESOLVE_DAMAGE_HOSTILE));
	eq(streq(vanguard_resolve_status_name(&p), "Resolve: Resolute"), true);
	p.chp = 10;
	eq(streq(vanguard_resolve_status_name(&p), "Resolve: Last Stand"), true);
	vanguard_resolve_clear(&p);
	eq(streq(vanguard_resolve_status_name(&p), "Resolve: Unbroken"), true);

	ok;
}

static int test_deep_vanguard_playtest(void *state)
{
	if (vanguard_scenario_only()) ok;

	require(prepare_high_level_vanguard());
	require(assert_vanguard_resolve_savefile_round_trip() == 0);
	require(assert_order_list_is_clean() == 0);
	require(assert_resolve_orders_activate_clean_buffs() == 0);
	require(assert_hostile_monster_damage_builds_pressure() == 0);
	require(assert_resolve_passive_and_first_orders_scale() == 0);
	require(assert_remaining_orders_scale_and_utilities_do_not() == 0);
	require(assert_frontline_orders_affect_adjacent_enemy() == 0);

	ok;
}

static int test_vanguard_scenario_save(void *state)
{
	const char *scenario_id = getenv("HEROBAND_VANGUARD_SCENARIO_ID");
	const char *scenario_save = getenv("HEROBAND_VANGUARD_SCENARIO_SAVE_OUT");
	const char *scenario_report =
		getenv("HEROBAND_VANGUARD_SCENARIO_REPORT_OUT");
	const char *level_text = getenv("HEROBAND_VANGUARD_LEVEL");
	const char *depth_text = getenv("HEROBAND_VANGUARD_DEPTH");
	const struct vanguard_scenario *scenario;
	const char *save_path;
	int expected_level;
	int expected_depth;

	if (!scenario_id || !scenario_id[0]) {
		ok;
	}

	scenario = vanguard_scenario_by_id(scenario_id);
	require(scenario != NULL);
	save_path = scenario_save && scenario_save[0] ?
		scenario_save : "VanguardScenarioTest";
	expected_level = level_text && level_text[0] ? atoi(level_text) :
		scenario->level;
	expected_depth = depth_text && depth_text[0] ? atoi(depth_text) :
		scenario->depth;

	require(prepare_vanguard_scenario(scenario));
	eq(player->is_dead, false);
	eq(player->lev, expected_level);
	eq(player->depth, expected_depth);
	require(player->class != NULL);
	require(streq(player->class->name, "Vanguard"));
	require(savefile_save(save_path));
	require(file_exists(save_path));
	write_vanguard_scenario_report(scenario_report, scenario);

	ok;
}

const char *suite_name = "game/vanguard";
struct test tests[] = {
	{ "vanguard_scenario_save", test_vanguard_scenario_save },
	{ "heroic_resolve_tiers", test_heroic_resolve_tiers },
	{ "heroic_resolve_pressure_sources",
		test_heroic_resolve_pressure_sources },
	{ "heroic_resolve_decay_and_cleanup",
		test_heroic_resolve_decay_and_cleanup },
	{ "heroic_resolve_status_names", test_heroic_resolve_status_names },
	{ "armor_mastery_requires_clean_shield_or_heavy_armor",
		test_armor_mastery_requires_clean_shield_or_heavy_armor },
	{ "deep_vanguard_playtest", test_deep_vanguard_playtest },
	{ NULL, NULL }
};
