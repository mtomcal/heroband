/*
 * effects/general
 * Exercise General class tactical effects.
 */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "cmd-core.h"
#include "effect-handler.h"
#include "effect-handler-general.h"
#include "game-world.h"
#include "init.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "monster.h"
#include "player.h"
#include "player-birth.h"
#include "player-timed.h"
#include "player-util.h"
#include "source.h"

int setup_tests(void **state)
{
	set_file_paths();
	init_angband();
	*state = NULL;
	return 0;
}

static int count_called_allies(struct monster **called_ally)
{
	int count = 0;
	int i;

	*called_ally = NULL;
	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);

		if (!mon->race) {
			continue;
		}
		if (mflag_has(mon->mflag, MFLAG_CALLED_ALLY)) {
			*called_ally = mon;
			++count;
		}
	}

	return count;
}

static int count_live_monsters(void)
{
	int count = 0;
	int i;

	for (i = 1; i < cave_monster_max(cave); ++i) {
		struct monster *mon = cave_monster(cave, i);

		if (mon->race) {
			++count;
		}
	}

	return count;
}

static int count_floor_objects(void)
{
	int count = 0;
	int i;

	for (i = 1; i < cave->obj_max; ++i) {
		if (cave->objects[i]) {
			++count;
		}
	}

	return count;
}

static void clear_command_state_for_test(void)
{
	if (player && player->timed) {
		player->timed[TMD_COMMAND] = 0;
	}
}

static void setup_known_cave(struct chunk *c, struct player *p)
{
	int i;

	p->cave = cave_new(c->height, c->width);
	p->cave->objects = mem_realloc(p->cave->objects, (c->obj_max + 1) *
		sizeof(struct object*));
	p->cave->obj_max = c->obj_max;
	for (i = 0; i <= p->cave->obj_max; ++i) {
		p->cave->objects[i] = NULL;
	}
	p->cave->depth = c->depth;
}

static struct player_class *lookup_class_name(const char *name)
{
	struct player_class *c;

	for (c = classes; c; c = c->next) {
		if (streq(c->name, name)) {
			return c;
		}
	}
	return NULL;
}

static const struct class_spell *lookup_class_spell(
	const struct player_class *c, const char *name)
{
	int i;

	for (i = 0; i < c->magic.num_books; ++i) {
		const struct class_book *book = &c->magic.books[i];
		int j;

		for (j = 0; j < book->num_spells; ++j) {
			const struct class_spell *spell = &book->spells[j];

			if (spell->name && streq(spell->name, name)) {
				return spell;
			}
		}
	}

	return NULL;
}

static bool effect_chain_has_index(const struct effect *effect, int index)
{
	while (effect) {
		if (effect->index == index) {
			return true;
		}
		effect = effect->next;
	}
	return false;
}

static bool general_spell_effect_chain_is_clean(const struct effect *effect)
{
	while (effect) {
		switch (effect->index) {
			case EF_CALL_ALLY:
				if (effect->subtype != 1 && effect->subtype != 2) {
					return false;
				}
				break;
			case EF_CURE:
				if (effect->subtype != TMD_AFRAID) {
					return false;
				}
				break;
			case EF_TIMED_INC:
				if (effect->subtype != TMD_HERO &&
						effect->subtype != TMD_SHIELD &&
						effect->subtype != TMD_BLESSED) {
					return false;
				}
				break;
			case EF_GENERAL_FORMATION:
				if (effect->subtype < GENERAL_FORMATION_ARROW_VOLLEY ||
						effect->subtype > GENERAL_FORMATION_MARSHALS_BANNER) {
					return false;
				}
				break;
			default:
				return false;
		}
		effect = effect->next;
	}
	return true;
}

static bool text_has_any_term(const char *text, const char **terms, size_t n_terms)
{
	size_t i;

	if (!text) {
		return false;
	}
	for (i = 0; i < n_terms; ++i) {
		if (my_stristr(text, terms[i])) {
			return true;
		}
	}
	return false;
}

int teardown_tests(void *state)
{
	cleanup_angband();
	return 0;
}

static int test_low_level_infantry_uses_in_world_race(void *state)
{
	struct monster_race *race = general_ally_race(1, 1);

	require(race != NULL);
	require(streq(race->name, "Westfold footman"));
	eq(race->mexp, 0);
	ok;
}

static int test_high_level_infantry_uses_stronger_tier(void *state)
{
	struct monster_race *race = general_ally_race(1, 30);

	require(race != NULL);
	require(streq(race->name, "Gondor captain"));
	eq(race->mexp, 0);
	ok;
}

static int test_archer_uses_in_world_tier(void *state)
{
	struct monster_race *race = general_ally_race(2, 7);
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;

	require(race != NULL);
	require(streq(race->name, "Ithilien bowman"));
	eq(race->mexp, 0);
	require(general != NULL);
	spell = lookup_class_spell(general, "Call Archer");
	require(spell != NULL);
	require(spell->slevel <= 7);
	require(spell->smana <= 5);
	require(spell->sfail <= 10);
	ok;
}

static int test_high_level_archer_uses_stronger_tier(void *state)
{
	struct monster_race *race = general_ally_race(2, 30);

	require(race != NULL);
	require(streq(race->name, "Citadel marksman"));
	eq(race->mexp, 0);
	ok;
}

static int test_temporary_ally_tiers_do_not_use_project_labels(void *state)
{
	int subtypes[] = { 1, 2 };
	int levels[] = { 1, 30 };
	size_t i;
	size_t j;

	for (i = 0; i < N_ELEMENTS(subtypes); ++i) {
		for (j = 0; j < N_ELEMENTS(levels); ++j) {
			struct monster_race *race = general_ally_race(subtypes[i],
				levels[j]);

			require(race != NULL);
			require(strncmp(race->name, "Heroband ", 9) != 0);
			eq(race->mexp, 0);
		}
	}
	ok;
}

static int test_call_ally_creates_one_commanded_living_soldier(void *state)
{
	effect_handler_context_t context = {
		.effect = EF_CALL_ALLY,
		.origin = { SRC_PLAYER, { 0 } },
		.aware = true,
		.value = { .base = 12 },
		.subtype = 1
	};
	struct monster *called_ally = NULL;

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player->lev = 30;
	player_clear_timed(player, TMD_COMMAND, false, false);

	require(effect_handler_CALL_ALLY(&context));
	eq(count_called_allies(&called_ally), 1);
	require(called_ally != NULL);
	require(streq(called_ally->race->name, "Gondor captain"));
	eq(player->timed[TMD_COMMAND], 12);
	eq(called_ally->m_timed[MON_TMD_COMMAND], 12);
	require(get_commanded_monster() == called_ally);
	eq(called_ally->race->mexp, 0);
	ok;
}

static int test_general_can_cast_while_commanding_ally(void *state)
{
	struct command cast = {
		.context = CTX_GAME,
		.code = CMD_CAST,
		.nrepeats = 0,
		.background_command = 0,
		.arg = { { 0 } }
	};
	struct command walk = {
		.context = CTX_GAME,
		.code = CMD_WALK,
		.nrepeats = 0,
		.background_command = 0,
		.arg = { { 0 } }
	};
	struct command sleep = {
		.context = CTX_GAME,
		.code = CMD_SLEEP,
		.nrepeats = 0,
		.background_command = 0,
		.arg = { { 0 } }
	};
	struct player_class *general = lookup_class_name("General");
	const struct player_class *original_class;

	require(general != NULL);
	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	original_class = player->class;
	player->class = general;
	player->timed[TMD_COMMAND] = 10;

	require(!cmd_command_mode_redirects_command(&cast));
	require(cmd_command_mode_redirects_command(&walk));
	require(!cmd_command_mode_redirects_command(&sleep));
	player->class = original_class;
	ok;
}

static int test_fighting_withdrawal_replaces_general_teleport_data(void *state)
{
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;

	require(general != NULL);
	require(lookup_class_spell(general, "Tactical Withdrawal") == NULL);
	spell = lookup_class_spell(general, "Fighting Withdrawal");
	require(spell != NULL);
	require(spell->slevel <= 7);
	require(spell->smana <= 3);
	require(!effect_chain_has_index(spell->effect, EF_TELEPORT));
	ok;
}

static int test_fighting_withdrawal_defends_without_relocation(void *state)
{
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;
	struct loc start;
	bool ident = false;
	bool completed = false;

	require(general != NULL);
	spell = lookup_class_spell(general, "Fighting Withdrawal");
	require(spell != NULL);

	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player_clear_timed(player, TMD_SHIELD, false, false);
	player_clear_timed(player, TMD_BLESSED, false, false);
	start = player->grid;

	completed = effect_do(spell->effect, source_player(), NULL, &ident,
		true, 0, 0, false, NULL);

	require(completed);
	require(loc_eq(player->grid, start));
	require(player->timed[TMD_SHIELD] > 0);
	require(player->timed[TMD_BLESSED] > 0);
	ok;
}

static int test_arrow_volley_disrupts_without_extra_monsters(void *state)
{
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;
	struct monster *near_mon;
	struct monster *far_mon;
	struct monster *called_ally = NULL;
	bool ident = false;
	int before_count;
	int before_called;
	int before_objects;

	require(general != NULL);
	spell = lookup_class_spell(general, "Arrow Volley");
	require(spell != NULL);
	require(spell->slevel <= 15);
	require(spell->smana <= 7);
	require(spell->sfail <= 10);

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player->lev = 15;
	player_clear_timed(player, TMD_COMMAND, false, false);
	near_mon = t_add_monster(cave, loc(11, 10), "kobold");
	far_mon = t_add_monster(cave, loc(16, 10), "kobold");
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();

	require(effect_do(spell->effect, source_player(), NULL, &ident, true,
		0, 0, false, NULL));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	require(near_mon->m_timed[MON_TMD_SLOW] > 0);
	eq(far_mon->m_timed[MON_TMD_SLOW], 0);
	require(get_commanded_monster() == NULL);
	ok;
}

static int arrow_volley_slow_duration_with_optional_archer(bool with_archer)
{
	effect_handler_context_t context = {
		.effect = EF_GENERAL_FORMATION,
		.origin = { SRC_PLAYER, { 0 } },
		.aware = true,
		.value = { .base = 5 },
		.subtype = GENERAL_FORMATION_ARROW_VOLLEY,
		.radius = 3
	};
	struct monster *enemy;
	struct monster *called_ally = NULL;
	int before_count;
	int before_called;
	int before_objects;

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player->lev = 15;

	if (with_archer) {
		struct monster *archer = t_add_monster(cave, loc(9, 10),
			"Ithilien bowman");

		mflag_on(archer->mflag, MFLAG_CALLED_ALLY);
		archer->m_timed[MON_TMD_COMMAND] = 20;
		player->timed[TMD_COMMAND] = 20;
	}

	enemy = t_add_monster(cave, loc(11, 10), "kobold");
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();
	require(effect_handler_GENERAL_FORMATION(&context));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	return enemy->m_timed[MON_TMD_SLOW];
}

static int test_arrow_volley_is_stronger_with_archer_line_ally(void *state)
{
	int solo_duration = arrow_volley_slow_duration_with_optional_archer(false);
	int archer_duration = arrow_volley_slow_duration_with_optional_archer(true);

	require(solo_duration > 0);
	require(archer_duration > solo_duration);
	ok;
}

static int test_arrow_volley_does_not_trivially_disable_unique(void *state)
{
	effect_handler_context_t context = {
		.effect = EF_GENERAL_FORMATION,
		.origin = { SRC_PLAYER, { 0 } },
		.aware = true,
		.value = { .base = 20 },
		.subtype = GENERAL_FORMATION_ARROW_VOLLEY,
		.radius = 3
	};
	struct monster *unique;

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	unique = t_add_monster(cave, loc(11, 10), "Grip, Farmer Maggot's Dog");

	require(effect_handler_GENERAL_FORMATION(&context));
	eq(unique->m_timed[MON_TMD_SLOW], 0);
	ok;
}

static int test_glorious_charge_benefits_without_relocation(void *state)
{
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;
	struct loc start;
	bool ident = false;
	struct monster *called_ally = NULL;
	int before_count;
	int before_called;
	int before_objects;

	require(general != NULL);
	spell = lookup_class_spell(general, "Glorious Charge");
	require(spell != NULL);
	require(spell->slevel <= 20);
	require(spell->smana <= 8);
	require(spell->sfail <= 10);

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player->lev = 20;
	player_clear_timed(player, TMD_HERO, false, false);
	player_clear_timed(player, TMD_BLESSED, false, false);
	start = player->grid;
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();

	require(effect_do(spell->effect, source_player(), NULL, &ident, true,
		0, 0, false, NULL));
	require(loc_eq(player->grid, start));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	require(player->timed[TMD_HERO] > 0);
	require(player->timed[TMD_BLESSED] > 0);
	ok;
}

static int glorious_charge_slow_duration_with_optional_melee(bool with_melee)
{
	effect_handler_context_t context = {
		.effect = EF_GENERAL_FORMATION,
		.origin = { SRC_PLAYER, { 0 } },
		.aware = true,
		.value = { .base = 5 },
		.subtype = GENERAL_FORMATION_GLORIOUS_CHARGE,
		.radius = 2
	};
	struct monster *enemy;
	struct monster *called_ally = NULL;
	int before_count;
	int before_called;
	int before_objects;

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player->lev = 20;

	if (with_melee) {
		struct monster *infantry = t_add_monster(cave, loc(9, 10),
			"Westfold footman");

		mflag_on(infantry->mflag, MFLAG_CALLED_ALLY);
		infantry->m_timed[MON_TMD_COMMAND] = 20;
		player->timed[TMD_COMMAND] = 20;
	}

	enemy = t_add_monster(cave, loc(11, 10), "kobold");
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();
	require(effect_handler_GENERAL_FORMATION(&context));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	return enemy->m_timed[MON_TMD_SLOW];
}

static int test_glorious_charge_disrupts_nearby_nonunique_only(void *state)
{
	effect_handler_context_t context = {
		.effect = EF_GENERAL_FORMATION,
		.origin = { SRC_PLAYER, { 0 } },
		.aware = true,
		.value = { .base = 5 },
		.subtype = GENERAL_FORMATION_GLORIOUS_CHARGE,
		.radius = 2
	};
	struct monster *near_mon;
	struct monster *far_mon;
	struct monster *called_ally = NULL;
	int before_count;
	int before_called;
	int before_objects;

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	near_mon = t_add_monster(cave, loc(11, 10), "kobold");
	far_mon = t_add_monster(cave, loc(14, 10), "kobold");
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();

	require(effect_handler_GENERAL_FORMATION(&context));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	require(near_mon->m_timed[MON_TMD_SLOW] > 0);
	eq(far_mon->m_timed[MON_TMD_SLOW], 0);
	ok;
}

static int test_glorious_charge_is_stronger_with_melee_line_ally(void *state)
{
	int solo_duration = glorious_charge_slow_duration_with_optional_melee(false);
	int melee_duration = glorious_charge_slow_duration_with_optional_melee(true);

	require(solo_duration > 0);
	require(melee_duration > solo_duration);
	ok;
}

static int test_marshals_banner_creates_fixed_zone(void *state)
{
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;
	struct loc start;
	bool ident = false;
	struct monster *called_ally = NULL;
	int before_count;
	int before_called;
	int before_objects;

	require(general != NULL);
	spell = lookup_class_spell(general, "Marshal's Banner");
	require(spell != NULL);
	require(spell->slevel <= 20);
	require(spell->smana <= 8);
	require(spell->sfail <= 10);

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	start = player->grid;
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();

	require(effect_do(spell->effect, source_player(), NULL, &ident, true,
		0, 0, false, NULL));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	require(player->general_banner.active);
	require(loc_eq(player->general_banner.grid, start));
	eq(player->general_banner.radius, 3);
	require(player->general_banner.duration > 0);
	ok;
}

static int test_marshals_banner_benefits_only_inside_zone(void *state)
{
	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	player->general_banner.active = true;
	player->general_banner.grid = loc(10, 10);
	player->general_banner.radius = 3;
	player->general_banner.duration = 3;
	player_clear_timed(player, TMD_HERO, false, false);
	player_clear_timed(player, TMD_BLESSED, false, false);

	general_banner_tick(player);
	require(player->timed[TMD_HERO] > 0);
	require(player->timed[TMD_BLESSED] > 0);

	player_clear_timed(player, TMD_HERO, false, false);
	player_clear_timed(player, TMD_BLESSED, false, false);
	player_place(cave, player, loc(15, 10));
	general_banner_tick(player);
	eq(player->timed[TMD_HERO], 0);
	eq(player->timed[TMD_BLESSED], 0);
	ok;
}

static int test_marshals_banner_disrupts_monsters_inside_radius_only(void *state)
{
	struct player_class *general = lookup_class_name("General");
	const struct class_spell *spell;
	struct monster *near_mon;
	struct monster *far_mon;
	bool ident = false;
	struct monster *called_ally = NULL;
	int before_count;
	int before_called;
	int before_objects;

	require(general != NULL);
	spell = lookup_class_spell(general, "Marshal's Banner");
	require(spell != NULL);

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, loc(10, 10));
	setup_known_cave(cave, player);
	near_mon = t_add_monster(cave, loc(11, 10), "kobold");
	far_mon = t_add_monster(cave, loc(15, 10), "kobold");
	before_count = count_live_monsters();
	before_called = count_called_allies(&called_ally);
	before_objects = count_floor_objects();

	require(effect_do(spell->effect, source_player(), NULL, &ident, true,
		0, 0, false, NULL));
	eq(count_live_monsters(), before_count);
	eq(count_called_allies(&called_ally), before_called);
	eq(count_floor_objects(), before_objects);
	require(near_mon->m_timed[MON_TMD_SLOW] > 0);
	eq(far_mon->m_timed[MON_TMD_SLOW], 0);
	ok;
}

static int test_marshals_banner_expiry_leaves_no_map_residue(void *state)
{
	struct loc banner_grid = loc(10, 10);
	int original_feat;

	clear_command_state_for_test();
	player_make_simple(NULL, NULL, "Tester");
	cave = t_build_arena(20, 20);
	player_place(cave, player, banner_grid);
	setup_known_cave(cave, player);
	original_feat = square(cave, banner_grid)->feat;
	player->general_banner.active = true;
	player->general_banner.grid = banner_grid;
	player->general_banner.radius = 3;
	player->general_banner.duration = 1;

	general_banner_tick(player);
	require(!player->general_banner.active);
	eq(square(cave, banner_grid)->feat, original_feat);
	require(square_object(cave, banner_grid) == NULL);
	ok;
}

static int test_general_player_facing_text_uses_clean_marshal_language(void *state)
{
	static const char *forbidden_terms[] = {
		"Heroband ",
		"Tactical Withdrawal",
		"Banner of Courage",
		"teleport",
		"summon",
		"necromancy",
		"undead",
		"demon",
		"blood",
		"occult",
		"shadow",
	};
	struct player_class *general = lookup_class_name("General");
	int i;

	require(general != NULL);
	for (i = 0; i < general->magic.num_books; ++i) {
		const struct class_book *book = &general->magic.books[i];
		int j;

		for (j = 0; j < book->num_spells; ++j) {
			const struct class_spell *spell = &book->spells[j];

			require(!text_has_any_term(spell->name, forbidden_terms,
				N_ELEMENTS(forbidden_terms)));
			require(!text_has_any_term(spell->text, forbidden_terms,
				N_ELEMENTS(forbidden_terms)));
			require(!effect_chain_has_index(spell->effect, EF_TELEPORT));
			require(!effect_chain_has_index(spell->effect, EF_SUMMON));
			require(general_spell_effect_chain_is_clean(spell->effect));
		}
	}
	require(lookup_monster("Heroband infantry") == NULL);
	require(lookup_monster("Heroband archer") == NULL);
	ok;
}

const char *suite_name = "effects/general";
struct test tests[] = {
	{ "low_level_infantry_uses_in_world_race",
		test_low_level_infantry_uses_in_world_race },
	{ "high_level_infantry_uses_stronger_tier",
		test_high_level_infantry_uses_stronger_tier },
	{ "archer_uses_in_world_tier", test_archer_uses_in_world_tier },
	{ "high_level_archer_uses_stronger_tier",
		test_high_level_archer_uses_stronger_tier },
	{ "temporary_ally_tiers_do_not_use_project_labels",
		test_temporary_ally_tiers_do_not_use_project_labels },
	{ "call_ally_creates_one_commanded_living_soldier",
		test_call_ally_creates_one_commanded_living_soldier },
	{ "general_can_cast_while_commanding_ally",
		test_general_can_cast_while_commanding_ally },
	{ "fighting_withdrawal_replaces_general_teleport_data",
		test_fighting_withdrawal_replaces_general_teleport_data },
	{ "fighting_withdrawal_defends_without_relocation",
		test_fighting_withdrawal_defends_without_relocation },
	{ "arrow_volley_disrupts_without_extra_monsters",
		test_arrow_volley_disrupts_without_extra_monsters },
	{ "arrow_volley_is_stronger_with_archer_line_ally",
		test_arrow_volley_is_stronger_with_archer_line_ally },
	{ "arrow_volley_does_not_trivially_disable_unique",
		test_arrow_volley_does_not_trivially_disable_unique },
	{ "glorious_charge_benefits_without_relocation",
		test_glorious_charge_benefits_without_relocation },
	{ "glorious_charge_disrupts_nearby_nonunique_only",
		test_glorious_charge_disrupts_nearby_nonunique_only },
	{ "glorious_charge_is_stronger_with_melee_line_ally",
		test_glorious_charge_is_stronger_with_melee_line_ally },
	{ "marshals_banner_creates_fixed_zone",
		test_marshals_banner_creates_fixed_zone },
	{ "marshals_banner_benefits_only_inside_zone",
		test_marshals_banner_benefits_only_inside_zone },
	{ "marshals_banner_disrupts_monsters_inside_radius_only",
		test_marshals_banner_disrupts_monsters_inside_radius_only },
	{ "marshals_banner_expiry_leaves_no_map_residue",
		test_marshals_banner_expiry_leaves_no_map_residue },
	{ "general_player_facing_text_uses_clean_marshal_language",
		test_general_player_facing_text_uses_clean_marshal_language },
	{ NULL, NULL }
};
