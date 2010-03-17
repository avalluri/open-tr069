// $Id$

#include <string.h>
#include <stdio.h>

#include "class_xml.h"

#include "test_suite.h"

static struct evbuffer *buffer;
static struct evcpe_class *cls;

static void test_setup(void)
{
	buffer = evbuffer_new();
	cls = evcpe_class_new(NULL);
}

static void test_teardown(void)
{
	evcpe_class_free(cls);
	evbuffer_free(buffer);
}

static void test_init(void)
{
	TEST_ASSERT_NOT_NULL(buffer);
	TEST_ASSERT_NOT_NULL(cls);
}

static void test_load(const char *filename)
{
	FILE *file;
	int fd, len;

	TEST_ASSERT_NOT_NULL((file = fopen(filename, "r")));
	fd = fileno(file);
	while ((len = evbuffer_read(buffer, fd, -1)));
	fclose(file);
}

static void test_tr098(void)
{
	test_load("testfiles/tr098_model.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_class_from_xml(cls, buffer));
}

TestRef evcpe_model_xml_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("model_xml_test_init", test_init),
		new_TestFixture("model_xml_tr098", test_tr098),
	};
	EMB_UNIT_TESTCALLER(model_xml_test_case, "model_xml_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&model_xml_test_case;
}
