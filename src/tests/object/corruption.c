/* object/corruption.c */
/* Exercise corrupt object classification. */

#include "unit-test.h"
#include "init.h"
#include "obj-util.h"
#include "object.h"
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

const char *suite_name = "object/corruption";
struct test tests[] = {
	{ "one_ring_corrupt", test_one_ring_corrupt },
	{ "morgoth_artifacts_corrupt", test_morgoth_artifacts_corrupt },
	{ "morgul_ego_corrupt", test_morgul_ego_corrupt },
	{ "clean_items_not_corrupt", test_clean_items_not_corrupt },
	{ "object_corruption_from_artifact_or_ego",
		test_object_corruption_from_artifact_or_ego },
	{ NULL, NULL }
};
