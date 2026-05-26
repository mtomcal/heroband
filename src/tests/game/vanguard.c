/* game/vanguard.c */

#include "unit-test.h"
#include "test-utils.h"

#include <stdio.h>
#include <string.h>
#include "cave.h"
#include "effects.h"
#include "game-event.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "monster.h"
#include "mon-make.h"
#include "player.h"
#include "player-birth.h"
#include "player-spell.h"
#include "player-timed.h"
#include "z-util.h"

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
	wipe_mon_list(cave, player);
	cleanup_angband();
	return 0;
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
	require(cast_until_active("Stand Firm", TMD_SHIELD));
	require(player->timed[TMD_HERO] > 0);

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

	if (mon->midx == 0) {
		mon = t_add_monster(cave, loc(11, 10), "kobold");
	}
	old_hp = mon->hp;
	player->csp = player->msp;
	require(spell_cast(find_spell("Forceful Blow"), DIR_E, NULL));
	require(mon->hp < old_hp || mon->midx == 0);
	return 0;
}

static int test_deep_vanguard_playtest(void *state)
{
	require(prepare_high_level_vanguard());
	require(assert_order_list_is_clean() == 0);
	require(assert_resolve_orders_activate_clean_buffs() == 0);
	require(assert_frontline_orders_affect_adjacent_enemy() == 0);

	ok;
}

const char *suite_name = "game/vanguard";
struct test tests[] = {
	{ "deep_vanguard_playtest", test_deep_vanguard_playtest },
	{ NULL, NULL }
};
