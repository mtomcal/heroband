/* game/general.c */

#include "unit-test.h"
#include "test-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cave.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "mon-util.h"
#include "monster.h"
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
#include "savefile.h"
#include "z-util.h"

struct general_scenario {
	const char *id;
	const char *save_name;
	int level;
	int depth;
	const char *ally_mode;
	bool banner_active;
	bool include_unique;
	bool include_evil;
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
	file_delete("GeneralScenarioTest");
	wipe_mon_list(cave, player);
	cleanup_angband();
	return 0;
}

static const struct general_scenario *general_scenario_by_id(const char *id)
{
	static const struct general_scenario scenarios[] = {
		{ "general-l01-smoke", "general-l01-smoke", 1, 1, "low infantry",
			false, false, false },
		{ "general-l07-archer-withdrawal",
			"general-l07-archer-withdrawal", 7, 7, "low archer",
			false, false, false },
		{ "general-l15-arrow-volley", "general-l15-arrow-volley", 15, 15,
			"archer-line formation", false, false, false },
		{ "general-l30-charge-banner", "general-l30-charge-banner", 30, 30,
			"melee-line formation and banner", true, true, false },
		{ "general-l45-banner-cleanup", "general-l45-banner-cleanup", 45, 45,
			"high-tier cleanup", true, false, false },
		{ "general-moral-regression", "general-moral-regression", 35, 35,
			"moral regression", false, false, true },
		{ NULL, NULL, 0, 0, NULL, false, false, false }
	};
	int i;

	if (!id || !id[0]) {
		id = "general-l01-smoke";
	}
	for (i = 0; scenarios[i].id; ++i) {
		if (streq(scenarios[i].id, id)) {
			return &scenarios[i];
		}
	}
	return NULL;
}

static void learn_available_general_powers(void)
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

static void learn_general_power(const char *name)
{
	int i;

	for (i = 0; i < player->class->magic.total_spells; i++) {
		const struct class_spell *spell = spell_by_index(player, i);

		if (streq(spell->name, name)) {
			player->spell_flags[i] = PY_SPELL_LEARNED;
			player->spell_order[i] = i;
			return;
		}
	}
}

static bool add_general_book(int bidx)
{
	const struct class_book *book = &player->class->magic.books[bidx];
	struct object_kind *kind = lookup_kind(book->tval, book->sval);
	struct object *obj;

	if (!kind) {
		return false;
	}
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
		{ 2, 0 }, { -2, 0 }, { 0, 2 }, { 0, -2 },
		{ 2, 1 }, { 2, -1 }, { -2, 1 }, { -2, -1 },
		{ 3, 0 }, { -3, 0 }, { 0, 3 }, { 0, -3 }
	};
	struct monster_race *race = lookup_monster(race_name);
	struct monster_group_info info = { 0, 0 };
	size_t i;

	if (!race) {
		return false;
	}
	for (i = 0; i < N_ELEMENTS(offsets); ++i) {
		struct loc grid = loc(player->grid.x + preferred_dx + offsets[i][0],
			player->grid.y + preferred_dy + offsets[i][1]);

		if (!square_in_bounds(cave, grid)) {
			continue;
		}
		if (square_monster(cave, grid)) {
			delete_monster(cave, grid);
		}
		square_set_feat(cave, grid, FEAT_FLOOR);
		if (!square_isempty(cave, grid)) {
			continue;
		}
		if (place_new_monster(cave, grid, race, false, false, info,
				ORIGIN_DROP)) {
			return true;
		}
	}
	return false;
}

static bool add_extra_monsters_from_env(void)
{
	const char *names = getenv("HEROBAND_GENERAL_MONSTERS");
	char buf[512];
	char *name;
	int offset = 0;

	if (!names || !names[0]) {
		return true;
	}
	my_strcpy(buf, names, sizeof(buf));
	name = strtok(buf, ",");
	while (name) {
		while (*name == ' ') {
			++name;
		}
		if (!place_required_named_monster_near(name, 1 + (offset % 4),
				1 + (offset / 4))) {
			return false;
		}
		++offset;
		name = strtok(NULL, ",");
	}
	return true;
}

static bool add_scenario_monsters(const struct general_scenario *scenario)
{
	if (!place_required_named_monster_near("small kobold", 1, 0)) {
		return false;
	}
	if (!place_required_named_monster_near("kobold", 2, 0)) {
		return false;
	}
	if (!place_required_named_monster_near("kobold", 6, 0)) {
		return false;
	}
	if (scenario->include_unique) {
		if (!place_required_named_monster_near(
				"Grip, Farmer Maggot's Dog", 0, 2)) {
			return false;
		}
	}
	if (scenario->include_evil) {
		if (!place_required_named_monster_near("skeleton kobold", 1, 1)) {
			return false;
		}
		if (!place_required_named_monster_near("quasit", 2, 1)) {
			return false;
		}
	}
	return add_extra_monsters_from_env();
}

static bool equip_light_for_scenario(void)
{
	struct object_kind *kind = lookup_kind(TV_LIGHT, 1);
	struct object *obj;
	int slot;

	if (!kind) {
		return false;
	}
	obj = object_new();
	object_prep(obj, kind, 0, RANDOMISE);
	obj->number = 1;
	obj->known = object_new();
	object_set_base_known(player, obj);
	object_touch(player, obj);
	gear_insert_end(player, obj);
	player->upkeep->total_weight += object_weight_one(obj);
	slot = wield_slot(obj);
	if (slot < 0) {
		return false;
	}
	inven_wield(obj, slot);
	player->upkeep->update |= PU_BONUS | PU_TORCH;
	update_stuff(player);
	return player->state.cur_light > 0;
}

static void prepare_scenario_staging_area(void)
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

			if (!square_in_bounds(cave, grid)) {
				continue;
			}
			if (square_monster(cave, grid)) {
				delete_monster(cave, grid);
			}
			square_set_feat(cave, grid, FEAT_FLOOR);
		}
	}
}

static bool prepare_general_scenario(const struct general_scenario *scenario)
{
	const char *race = getenv("HEROBAND_GENERAL_RACE");
	const char *class_name = getenv("HEROBAND_GENERAL_CLASS");
	const char *level_text = getenv("HEROBAND_GENERAL_LEVEL");
	const char *depth_text = getenv("HEROBAND_GENERAL_DEPTH");
	int level = level_text && level_text[0] ? atoi(level_text) :
		scenario->level;
	int depth = depth_text && depth_text[0] ? atoi(depth_text) :
		scenario->depth;
	const char *banner_text = getenv("HEROBAND_GENERAL_BANNER");
	bool banner_active = banner_text && banner_text[0] ?
		!streq(banner_text, "0") : scenario->banner_active;

	if (!race || !race[0]) {
		race = "Human";
	}
	if (!class_name || !class_name[0]) {
		class_name = "General";
	}
	if (level < 1) {
		level = 1;
	}
	if (level > PY_MAX_LEVEL) {
		level = PY_MAX_LEVEL;
	}
	if (depth < 0) {
		depth = 0;
	}

	if (!player_make_simple(race, class_name, "Tester")) {
		return false;
	}
	player->depth = depth;
	prepare_next_level(player);
	on_new_level();
	prepare_scenario_staging_area();

	player->lev = level;
	player->max_lev = level;
	player->exp = player_exp[level - 1];
	player->max_exp = player->exp;
	player->mhp = MAX(player->mhp, 50 + level * 5);
	player->chp = player->mhp;
	player->msp = MAX(player->msp, 30 + level * 3);
	player->csp = player->msp;
	player->au = 1000;
	if (!equip_light_for_scenario()) {
		return false;
	}
	if (player->class->magic.num_books > 1 && !add_general_book(1)) {
		return false;
	}
	learn_available_general_powers();
	if (scenario->level >= 30) {
		learn_general_power("Fighting Withdrawal");
	}
	if (!add_scenario_monsters(scenario)) {
		return false;
	}

	if (streq(scenario->id, "general-l45-banner-cleanup")) {
		square_set_feat(cave, player->grid, FEAT_MORE);
	}

	if (banner_active) {
		player->general_banner.active = true;
		player->general_banner.grid = player->grid;
		player->general_banner.radius = 3;
		player->general_banner.duration = 20;
	}

	return true;
}

static void write_scenario_report(const char *path, const char *scenario_id)
{
	static const char *stat_names[] = { "STR", "INT", "WIS", "DEX", "CON" };
	ang_file *file;
	int i;

	if (!path || !path[0]) {
		return;
	}
	file = file_open(path, MODE_WRITE, FTYPE_TEXT);
	if (!file) {
		return;
	}
	file_putf(file, "## Exact Generated State\n\n");
	file_putf(file, "- Scenario ID: %s\n", scenario_id);
	file_putf(file, "- Name: %s\n", player->full_name);
	file_putf(file, "- Race: %s\n", player->race->name);
	file_putf(file, "- Class: %s\n", player->class->name);
	file_putf(file, "- Level: %d\n", player->lev);
	file_putf(file, "- Experience: %ld\n", (long) player->exp);
	file_putf(file, "- HP: %d/%d\n", player->chp, player->mhp);
	file_putf(file, "- SP: %d/%d\n", player->csp, player->msp);
	file_putf(file, "- Gold: %ld\n", (long) player->au);
	file_putf(file, "- Stats:\n");
	for (i = 0; i < STAT_MAX; ++i) {
		file_putf(file, "  - %s: current=%d max=%d use=%d top=%d\n",
			stat_names[i], player->stat_cur[i], player->stat_max[i],
			player->state.stat_use[i], player->state.stat_top[i]);
	}
	file_putf(file, "- Depth: %d\n", player->depth);
	file_putf(file, "- Starting grid: y=%d x=%d\n", player->grid.y,
		player->grid.x);
	file_putf(file, "- Light radius: %d\n", player->state.cur_light);
	file_putf(file, "- Banner active: %s\n",
		player->general_banner.active ? "yes" : "no");
	file_putf(file, "- Banner grid: y=%d x=%d\n",
		player->general_banner.grid.y, player->general_banner.grid.x);
	file_putf(file, "- Banner radius: %d\n", player->general_banner.radius);
	file_putf(file, "- Banner duration: %d\n",
		player->general_banner.duration);
	file_putf(file, "- RNG seed: normal game RNG; no stable external seed was pinned\n");
	file_putf(file, "- Expected formation fixture:\n");
	file_putf(file, "  - Radius origin: starting grid y=%d x=%d\n",
		player->grid.y, player->grid.x);
	file_putf(file, "  - Radius: 3 grids\n");
	file_putf(file, "  - In-radius nonunique actors expected affected by radius-3 formations:\n");
	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);
		int dist;

		if (!mon->race) {
			continue;
		}
		dist = distance(player->grid, mon->grid);
		if (dist <= 3 && !monster_is_unique(mon)) {
			file_putf(file, "    - %s at y=%d x=%d distance=%d unique=%s\n",
				mon->race->name, mon->grid.y, mon->grid.x, dist,
				monster_is_unique(mon) ? "yes" : "no");
		}
	}
	file_putf(file, "  - Actors expected unaffected by radius-3 formations:\n");
	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);
		int dist;

		if (!mon->race) {
			continue;
		}
		dist = distance(player->grid, mon->grid);
		if ((dist > 3 && dist <= 8) ||
				(dist <= 3 && monster_is_unique(mon))) {
			file_putf(file, "    - %s at y=%d x=%d distance=%d unique=%s\n",
				mon->race->name, mon->grid.y, mon->grid.x, dist,
				monster_is_unique(mon) ? "yes" : "no");
		}
	}
	file_putf(file, "  - Unique/resistant actors:\n");
	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);

		if (mon->race && monster_is_unique(mon)) {
			file_putf(file, "    - %s at y=%d x=%d distance=%d\n",
				mon->race->name, mon->grid.y, mon->grid.x,
				distance(player->grid, mon->grid));
		}
	}
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
	file_putf(file, "- Learned powers:\n");
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
			file_putf(file, "  - %s at y=%d x=%d awake=%s unique=%s\n",
				mon->race->name, mon->grid.y, mon->grid.x,
				mon->m_timed[MON_TMD_SLEEP] ? "no" : "yes",
				monster_is_unique(mon) ? "yes" : "no");
		}
	}
	file_close(file);
}

static int test_general_scenario_save(void *state)
{
	const char *scenario_id = getenv("HEROBAND_GENERAL_SCENARIO_ID");
	const char *scenario_save = getenv("HEROBAND_GENERAL_SCENARIO_SAVE_OUT");
	const char *scenario_report = getenv("HEROBAND_GENERAL_SCENARIO_REPORT_OUT");
	const char *level_text = getenv("HEROBAND_GENERAL_LEVEL");
	const char *depth_text = getenv("HEROBAND_GENERAL_DEPTH");
	const struct general_scenario *scenario =
		general_scenario_by_id(scenario_id);
	const char *save_path = scenario_save && scenario_save[0] ?
		scenario_save : "GeneralScenarioTest";
	int expected_level = level_text && level_text[0] ? atoi(level_text) :
		scenario->level;
	int expected_depth = depth_text && depth_text[0] ? atoi(depth_text) :
		scenario->depth;

	require(scenario != NULL);
	require(prepare_general_scenario(scenario));
	eq(player->is_dead, false);
	eq(player->lev, expected_level);
	eq(player->depth, expected_depth);
	require(player->class != NULL);
	require(streq(player->class->name, "General"));
	require(savefile_save(save_path));
	require(file_exists(save_path));
	write_scenario_report(scenario_report, scenario->id);

	ok;
}

const char *suite_name = "game/general";
struct test tests[] = {
	{ "general_scenario_save", test_general_scenario_save },
	{ NULL, NULL }
};
