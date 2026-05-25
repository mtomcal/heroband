/* object/corruption.c */
/* Exercise corrupt object classification. */

#include "unit-test.h"
#include "effects.h"
#include "init.h"
#include "obj-util.h"
#include "object.h"
#include "player-timed.h"
#include "test-utils.h"
#include "z-virt.h"

int setup_tests(void **state)
{
	set_file_paths();
	init_angband();
#ifdef UNIX
	create_needed_dirs();
#endif
	*state = NULL;
	return 0;
}

int teardown_tests(void *state)
{
	cleanup_angband();
	return 0;
}

static int test_one_ring_corrupt(void *state)
{
	const struct artifact *art = lookup_artifact_name("The One Ring");

	notnull(art);
	require(artifact_is_corrupt(art));
	ok;
}

static int test_morgoth_artifacts_corrupt(void *state)
{
	const struct artifact *grond = lookup_artifact_name("Grond");
	const struct artifact *crown = lookup_artifact_name("of Morgoth");

	notnull(grond);
	notnull(crown);
	require(artifact_is_corrupt(grond));
	require(artifact_is_corrupt(crown));
	ok;
}

static int test_morgul_ego_corrupt(void *state)
{
	int dagger_sval = lookup_sval(TV_SWORD, "Dagger");
	struct ego_item *ego = lookup_ego_item("of Morgul", TV_SWORD, dagger_sval);

	notnull(ego);
	require(ego_is_corrupt(ego));
	ok;
}

static int test_clean_items_not_corrupt(void *state)
{
	int dagger_sval = lookup_sval(TV_SWORD, "Dagger");
	const struct artifact *art = lookup_artifact_name("Narthanc");
	struct ego_item *ego = lookup_ego_item("of Flame", TV_SWORD,
		dagger_sval);
	struct object obj = { 0 };

	notnull(art);
	notnull(ego);

	require(!artifact_is_corrupt(art));
	require(!ego_is_corrupt(ego));
	require(!object_is_corrupt(&obj));
	ok;
}

static int test_object_corruption_from_artifact_or_ego(void *state)
{
	int dagger_sval = lookup_sval(TV_SWORD, "Dagger");
	struct object artifact_obj = { 0 };
	struct object ego_obj = { 0 };

	artifact_obj.artifact = lookup_artifact_name("The One Ring");
	ego_obj.ego = lookup_ego_item("of Morgul", TV_SWORD, dagger_sval);
	notnull(artifact_obj.artifact);
	notnull(ego_obj.ego);

	require(object_is_corrupt(&artifact_obj));
	require(object_is_corrupt(&ego_obj));
	ok;
}

static bool effect_chain_has_mon_drain(const struct effect *effect)
{
	int mon_drain = effect_subtype(EF_BOLT_STATUS_DAM, "MON_DRAIN");

	while (effect) {
		if ((effect->index == EF_BOLT_STATUS_DAM ||
				effect->index == EF_BOLT) &&
				effect->subtype == mon_drain) {
			return true;
		}
		effect = effect->next;
	}

	return false;
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

static int test_player_devices_do_not_expose_drain_life(void *state)
{
	require(lookup_sval(TV_WAND, "Drain Life") < 0);
	require(lookup_sval(TV_ROD, "Drain Life") < 0);
	ok;
}

static int test_player_devices_do_not_use_mon_drain(void *state)
{
	int i;

	for (i = 0; i < z_info->k_max; i++) {
		const struct object_kind *kind = &k_info[i];

		if (!kind->name || (kind->tval != TV_WAND && kind->tval != TV_ROD)) {
			continue;
		}

		require(!effect_chain_has_mon_drain(kind->effect));
	}

	ok;
}

static int test_drain_life_artifacts_corrupt_gated(void *state)
{
	const struct artifact *turmil = lookup_artifact_name("'Turmil'");
	const struct artifact *theoden = lookup_artifact_name("of Théoden");

	notnull(turmil);
	notnull(theoden);
	require(artifact_is_corrupt(turmil));
	require(artifact_is_corrupt(theoden));
	ok;
}

static int test_forbidden_scrolls_have_no_normal_allocation(void *state)
{
	const char *names[] = {
		"Summon Undead",
		"Curse Weapon",
		"Curse Armour"
	};
	size_t i;

	for (i = 0; i < N_ELEMENTS(names); i++) {
		int sval = lookup_sval(TV_SCROLL, names[i]);
		struct object_kind *kind;

		require(sval >= 0);
		kind = lookup_kind(TV_SCROLL, sval);
		notnull(kind);
		require(kind->alloc_prob == 0);
	}

	ok;
}

static int test_normal_objects_do_not_grant_vampirism_or_bloodlust(void *state)
{
	int i;

	for (i = 0; i < z_info->k_max; i++) {
		const struct object_kind *kind = &k_info[i];

		if (!kind->name || kind->alloc_prob == 0) {
			continue;
		}

		require(!effect_chain_timed_inc(kind->effect, TMD_ATT_VAMP));
		require(!effect_chain_timed_inc(kind->effect, TMD_BLOODLUST));
	}

	ok;
}

const char *suite_name = "object/corruption";
struct test tests[] = {
	{ "one_ring_corrupt", test_one_ring_corrupt },
	{ "morgoth_artifacts_corrupt", test_morgoth_artifacts_corrupt },
	{ "morgul_ego_corrupt", test_morgul_ego_corrupt },
	{ "clean_items_not_corrupt", test_clean_items_not_corrupt },
	{ "object_corruption_from_artifact_or_ego",
		test_object_corruption_from_artifact_or_ego },
	{ "player_devices_do_not_expose_drain_life",
		test_player_devices_do_not_expose_drain_life },
	{ "player_devices_do_not_use_mon_drain",
		test_player_devices_do_not_use_mon_drain },
	{ "drain_life_artifacts_corrupt_gated",
		test_drain_life_artifacts_corrupt_gated },
	{ "forbidden_scrolls_have_no_normal_allocation",
		test_forbidden_scrolls_have_no_normal_allocation },
	{ "normal_objects_do_not_grant_vampirism_or_bloodlust",
		test_normal_objects_do_not_grant_vampirism_or_bloodlust },
	{ NULL, NULL }
};
