/* player/corruption.c */
/* Exercise character-bound corruption from corrupt item use. */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "cmds.h"
#include "effects.h"
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "mon-make.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-quest.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "z-rand.h"
#include "z-virt.h"

static bool deny_corrupt_prompt(const char *prompt)
{
	return false;
}

static bool allow_corrupt_prompt(const char *prompt)
{
	return true;
}

static bool empty_gear(struct player *p)
{
	struct object *curr = p->gear;

	while (curr) {
		struct object *next = curr->next;
		bool none_left = false;

		if (object_is_equipped(p->body, curr)) {
			inven_takeoff(curr);
		}

		curr = gear_object_for_use(p, curr, curr->number, false,
			&none_left);
		if (curr->known) {
			object_free(curr->known);
		}
		object_free(curr);
		curr = next;
		if (!none_left) {
			return false;
		}
	}

	return pack_slots_used(p) == 0 && p->upkeep->equip_cnt == 0 &&
		p->upkeep->total_weight == 0;
}

static struct object *setup_object(int tval, int sval, int num)
{
	struct object_kind *kind = lookup_kind(tval, sval);
	struct object *obj = NULL;

	if (kind) {
		obj = object_new();
		object_prep(obj, kind, 0, RANDOMISE);
		obj->number = num;
		obj->known = object_new();
		object_set_base_known(player, obj);
		object_touch(player, obj);
	}
	return obj;
}

static struct activation *lookup_activation_name(const char *name)
{
	struct activation *act;

	for (act = &activations[1]; act; act = act->next) {
		if (act->name && streq(act->name, name)) {
			return act;
		}
	}
	return NULL;
}

static struct object *setup_corrupt_activatable_weapon(void)
{
	struct object *obj = setup_object(TV_SWORD, lookup_sval(TV_SWORD,
		"Dagger"), 1);

	if (!obj) {
		return NULL;
	}

	obj->ego = lookup_ego_item("of Morgul", TV_SWORD, obj->sval);
	obj->activation = lookup_activation_name("LIGHT");
	obj->time.base = 1;
	return obj;
}

static struct monster setup_final_quest_monster(void)
{
	struct monster mon = { 0 };
	struct monster_race *race = lookup_monster("Morgoth, Lord of Darkness");
	int i;

	if (!race || z_info->quest_max < 1) {
		return mon;
	}

	for (i = 0; i < z_info->quest_max; i++) {
		player->quests[i].level = 0;
		player->quests[i].cur_num = 0;
		player->quests[i].max_num = 1;
		player->quests[i].race = NULL;
	}

	player->quests[0].level = cave->depth;
	player->quests[0].cur_num = 0;
	player->quests[0].max_num = 1;
	player->quests[0].race = race;

	mon.race = race;
	mon.grid = player->grid;
	return mon;
}

static bool effect_chain_timed_inc(const struct effect *effect, int timed)
{
	while (effect) {
		if (effect->index == EF_TIMED_INC && effect->subtype == timed) {
			return true;
		}
		effect = effect->next;
	}

	return false;
}

int setup_tests(void **state)
{
	set_file_paths();
	init_angband();

	if (!player_make_simple(NULL, NULL, "Tester")) {
		cleanup_angband();
		return 1;
	}

	prepare_next_level(player);
	on_new_level();

	*state = NULL;
	return 0;
}

int teardown_tests(void *state)
{
	get_check_hook = NULL;
	wipe_mon_list(cave, player);
	cleanup_angband();
	return 0;
}

static int test_confirmed_corrupt_wield_increments_corruption(void *state)
{
	struct object *obj;
	struct command cmd = { 0 };

	require(empty_gear(player));
	player->corruption = 0;
	obj = setup_object(TV_RING, lookup_sval(TV_RING, "Searching"), 1);
	require(obj != NULL);
	obj->artifact = lookup_artifact_name("The One Ring");
	require(object_is_corrupt(obj));
	gear_insert_end(player, obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += object_weight_one(obj);

	get_check_hook = allow_corrupt_prompt;
	cmd.code = CMD_WIELD;
	cmd_set_arg_item(&cmd, "item", obj);
	do_cmd_wield(&cmd);
	get_check_hook = NULL;

	require(object_is_equipped(player->body, obj));
	require(player->corruption == 1);
	ok;
}

static int test_ordinary_cursed_wield_does_not_increment_corruption(void *state)
{
	struct object *obj;
	struct command cmd = { 0 };
	int curse = lookup_curse("irritation");

	require(empty_gear(player));
	player->corruption = 0;
	obj = setup_object(TV_BOOTS, lookup_sval(TV_BOOTS, "Pair of Leather Boots"),
		1);
	require(obj != NULL);
	require(curse > 0);
	require(append_object_curse(obj, curse, 10));
	require(!object_is_corrupt(obj));
	gear_insert_end(player, obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += object_weight_one(obj);

	cmd.code = CMD_WIELD;
	cmd_set_arg_item(&cmd, "item", obj);
	do_cmd_wield(&cmd);

	require(object_is_equipped(player->body, obj));
	require(player->corruption == 0);
	ok;
}

static int test_denied_corrupt_wield_does_not_increment_corruption(void *state)
{
	struct object *obj;
	struct command cmd = { 0 };

	require(empty_gear(player));
	player->corruption = 0;
	obj = setup_object(TV_RING, lookup_sval(TV_RING, "Searching"), 1);
	require(obj != NULL);
	obj->artifact = lookup_artifact_name("The One Ring");
	require(object_is_corrupt(obj));
	gear_insert_end(player, obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += object_weight_one(obj);

	get_check_hook = deny_corrupt_prompt;
	cmd.code = CMD_WIELD;
	cmd_set_arg_item(&cmd, "item", obj);
	do_cmd_wield(&cmd);
	get_check_hook = NULL;

	require(!object_is_equipped(player->body, obj));
	require(player->corruption == 0);
	ok;
}

static int test_confirmed_corrupt_activation_increments_corruption(void *state)
{
	struct object *obj;
	struct command cmd = { 0 };
	int slot;

	require(empty_gear(player));
	obj = setup_corrupt_activatable_weapon();
	require(obj != NULL);
	notnull(obj->ego);
	notnull(obj->activation);
	require(object_is_corrupt(obj));
	require(obj_can_activate(obj));
	gear_insert_end(player, obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += object_weight_one(obj);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	require(object_is_equipped(player->body, obj));
	player->corruption = 0;

	rand_fix(100);
	get_check_hook = allow_corrupt_prompt;
	cmd.code = CMD_ACTIVATE;
	cmd_set_arg_item(&cmd, "item", obj);
	cmd_set_arg_target(&cmd, "target", 2);
	do_cmd_activate(&cmd);
	get_check_hook = NULL;

	require(obj->timeout > 0);
	require(player->corruption == 1);
	ok;
}

static int test_denied_corrupt_activation_does_not_increment_corruption(void *state)
{
	struct object *obj;
	struct command cmd = { 0 };
	int slot;

	require(empty_gear(player));
	obj = setup_corrupt_activatable_weapon();
	require(obj != NULL);
	notnull(obj->ego);
	notnull(obj->activation);
	require(object_is_corrupt(obj));
	require(obj_can_activate(obj));
	gear_insert_end(player, obj);
	require(object_is_carried(player, obj));
	player->upkeep->total_weight += object_weight_one(obj);
	slot = wield_slot(obj);
	inven_wield(obj, slot);
	require(object_is_equipped(player->body, obj));
	player->corruption = 0;
	player->upkeep->energy_use = 0;

	get_check_hook = deny_corrupt_prompt;
	cmd.code = CMD_ACTIVATE;
	cmd_set_arg_item(&cmd, "item", obj);
	cmd_set_arg_target(&cmd, "target", 2);
	do_cmd_activate(&cmd);
	get_check_hook = NULL;

	require(obj->timeout == 0);
	require(player->upkeep->energy_use == 0);
	require(player->corruption == 0);
	ok;
}

static int test_low_corruption_has_no_hostile_threshold_effect(void *state)
{
	struct player_state local_state = player->state;

	player->corruption = PY_CORRUPTION_AGGRAVATE - 1;
	calc_bonuses(player, &local_state, false, false);

	require(!of_has(local_state.flags, OF_AGGRAVATE));
	require(!player_is_deeply_corrupted(player));
	ok;
}

static int test_middle_corruption_aggravates(void *state)
{
	struct player_state local_state = player->state;

	player->corruption = PY_CORRUPTION_AGGRAVATE;
	calc_bonuses(player, &local_state, false, false);

	require(of_has(local_state.flags, OF_AGGRAVATE));
	require(!player_is_deeply_corrupted(player));
	ok;
}

static int test_deep_corruption_marks_character_doomed(void *state)
{
	player->corruption = PY_CORRUPTION_DOOMED;

	require(player_is_deeply_corrupted(player));
	ok;
}

static int test_class_spells_do_not_grant_bloodlust(void *state)
{
	struct player_class *c;

	for (c = classes; c; c = c->next) {
		int i;

		for (i = 0; i < c->magic.num_books; i++) {
			const struct class_book *book = &c->magic.books[i];
			int j;

			for (j = 0; j < book->num_spells; j++) {
				const struct class_spell *spell = &book->spells[j];

				require(!effect_chain_timed_inc(spell->effect,
					TMD_BLOODLUST));
			}
		}
	}

	ok;
}

static int test_player_shapes_do_not_grant_vampirism_or_bloodlust(void *state)
{
	struct player_shape *shape;

	for (shape = shapes; shape; shape = shape->next) {
		require(!effect_chain_timed_inc(shape->effect, TMD_ATT_VAMP));
		require(!effect_chain_timed_inc(shape->effect, TMD_BLOODLUST));
	}

	ok;
}

static int test_vampire_and_werewolf_forms_not_player_available(void *state)
{
	require(lookup_player_shape("vampire") == NULL);
	require(lookup_player_shape("werewolf") == NULL);
	ok;
}

static int test_playable_classes_do_not_use_shadow_realm(void *state)
{
	struct player_class *c;

	for (c = classes; c; c = c->next) {
		int i;

		for (i = 0; i < c->magic.num_books; i++) {
			const struct class_book *book = &c->magic.books[i];

			require(book->realm == NULL || book->realm->code == NULL ||
				!streq(book->realm->code, "shadow"));
		}
	}

	ok;
}

static int test_stores_do_not_stock_or_buy_shadow_books(void *state)
{
	int i;

	for (i = 0; i < z_info->store_max; i++) {
		const struct store *store = &stores[i];
		const struct object_buy *buy;
		size_t j;

		for (j = 0; j < store->always_num; j++) {
			require(store->always_table[j]->tval != TV_SHADOW_BOOK);
		}

		for (j = 0; j < store->normal_num; j++) {
			require(store->normal_table[j]->tval != TV_SHADOW_BOOK);
		}

		for (buy = store->buy; buy; buy = buy->next) {
			require(buy->tval != TV_SHADOW_BOOK);
		}
	}

	ok;
}

static int test_monsters_do_not_drop_shadow_books(void *state)
{
	int i;

	for (i = 0; i < z_info->r_max; i++) {
		const struct monster_drop *drop;

		for (drop = r_info[i].drops; drop; drop = drop->next) {
			require(drop->tval != TV_SHADOW_BOOK);
			require(drop->kind == NULL ||
				drop->kind->tval != TV_SHADOW_BOOK);
		}
	}

	ok;
}

static int test_deep_corruption_blocks_heroic_victory(void *state)
{
	struct monster mon = setup_final_quest_monster();

	notnull(mon.race);
	player->corruption = PY_CORRUPTION_DOOMED;
	player->total_winner = false;
	player->is_dead = false;
	my_strcpy(player->died_from, "(alive and well)",
		sizeof(player->died_from));

	require(quest_check(player, &mon));
	require(!player->total_winner);
	ok;
}

static int test_deep_corruption_has_distinct_failure_reason(void *state)
{
	struct monster mon = setup_final_quest_monster();

	notnull(mon.race);
	player->corruption = PY_CORRUPTION_DOOMED;
	player->total_winner = false;
	player->is_dead = false;
	my_strcpy(player->died_from, "(alive and well)",
		sizeof(player->died_from));

	require(quest_check(player, &mon));
	require(player->is_dead);
	require(streq(player->died_from, "Corruption"));
	ok;
}

static int test_uncorrupted_final_quest_still_wins(void *state)
{
	struct monster mon = setup_final_quest_monster();

	notnull(mon.race);
	player->corruption = 0;
	player->total_winner = false;
	player->is_dead = false;
	my_strcpy(player->died_from, "(alive and well)",
		sizeof(player->died_from));

	require(quest_check(player, &mon));
	require(player->total_winner);
	require(!player->is_dead);
	ok;
}

const char *suite_name = "player/corruption";
struct test tests[] = {
	{ "confirmed_corrupt_wield_increments_corruption",
		test_confirmed_corrupt_wield_increments_corruption },
	{ "ordinary_cursed_wield_does_not_increment_corruption",
		test_ordinary_cursed_wield_does_not_increment_corruption },
	{ "denied_corrupt_wield_does_not_increment_corruption",
		test_denied_corrupt_wield_does_not_increment_corruption },
	{ "confirmed_corrupt_activation_increments_corruption",
		test_confirmed_corrupt_activation_increments_corruption },
	{ "denied_corrupt_activation_does_not_increment_corruption",
		test_denied_corrupt_activation_does_not_increment_corruption },
	{ "low_corruption_has_no_hostile_threshold_effect",
		test_low_corruption_has_no_hostile_threshold_effect },
	{ "middle_corruption_aggravates", test_middle_corruption_aggravates },
	{ "deep_corruption_marks_character_doomed",
		test_deep_corruption_marks_character_doomed },
	{ "class_spells_do_not_grant_bloodlust",
		test_class_spells_do_not_grant_bloodlust },
	{ "player_shapes_do_not_grant_vampirism_or_bloodlust",
		test_player_shapes_do_not_grant_vampirism_or_bloodlust },
	{ "vampire_and_werewolf_forms_not_player_available",
		test_vampire_and_werewolf_forms_not_player_available },
	{ "playable_classes_do_not_use_shadow_realm",
		test_playable_classes_do_not_use_shadow_realm },
	{ "stores_do_not_stock_or_buy_shadow_books",
		test_stores_do_not_stock_or_buy_shadow_books },
	{ "monsters_do_not_drop_shadow_books",
		test_monsters_do_not_drop_shadow_books },
	{ "deep_corruption_blocks_heroic_victory",
		test_deep_corruption_blocks_heroic_victory },
	{ "deep_corruption_has_distinct_failure_reason",
		test_deep_corruption_has_distinct_failure_reason },
	{ "uncorrupted_final_quest_still_wins",
		test_uncorrupted_final_quest_still_wins },
	{ NULL, NULL }
};
