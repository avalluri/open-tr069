// $Id$

#include "test_suite.h"

#include "cpe.h"

static struct event_base *evbase;
static struct evcpe *cpe;

static void test_setup(void)
{
	evbase = event_base_new();
	cpe = evcpe_new(evbase, NULL, NULL, NULL);
}

static void test_teardown(void)
{
	evcpe_free(cpe);
	event_base_free(evbase);
}

static void test_init(void)
{
	TEST_ASSERT_NOT_NULL(evbase);
	TEST_ASSERT_NOT_NULL(cpe);
}

TestRef evcpe_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("evcpe_test_init", test_init),
	};
	EMB_UNIT_TESTCALLER(evcpe_test_case, "evcpe_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&evcpe_test_case;
}
