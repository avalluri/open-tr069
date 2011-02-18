// $Id$
/*
 * Copyright (C) 2010 AXIM Communications Inc.
 * Copyright (C) 2010 Cedric Shih <cedric.shih@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include "test_suite.h"

#include "obj.h"

static struct evcpe_class *cls;
static struct evcpe_obj *obj;

static void test_setup(void)
{
	cls = evcpe_class_new(NULL);
}

static void test_teardown(void)
{
	evcpe_class_free(cls);
}

static void test_init(void)
{
	TEST_ASSERT_NOT_NULL(cls);
}

static void test_simple(void)
{
	const char *value;
	unsigned int len;
	struct evcpe_attr_schema *schema;
	struct evcpe_attr *attr;

	TEST_ASSERT_EQUAL_INT(0, evcpe_class_add(cls, &schema));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_schema_set_name(schema, "foo", strlen("foo")));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_schema_set_type(schema, EVCPE_TYPE_STRING));

	TEST_ASSERT_NOT_NULL((obj = evcpe_obj_new(cls, NULL)));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_init(obj));

	TEST_ASSERT(0 != evcpe_obj_get(obj, "bar", strlen("bar"), &attr));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_get(obj, "foo", strlen("foo"), &attr));
	TEST_ASSERT_NOT_NULL(attr);

	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_get(attr, &value, &len));
	TEST_ASSERT_NULL(value);
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_set(attr, "bar", strlen("bar")));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_get(attr, &value, &len));
	TEST_ASSERT_EQUAL_STRING("bar", value);
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_set(attr, "beque", strlen("beque")));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_get(attr, &value, &len));
	TEST_ASSERT_EQUAL_STRING("beque", value);
	evcpe_attr_unset(attr);
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_get(attr, &value, &len));
	TEST_ASSERT_NULL(value);
	evcpe_obj_free(obj);
}

static void test_obj(void)
{
	struct evcpe_obj *child;
	struct evcpe_attr_schema *schema;
	struct evcpe_attr *attr;
	const char *value;
	unsigned int len;

	TEST_ASSERT_EQUAL_INT(0, evcpe_class_add(cls, &schema));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_schema_set_name(schema, "foo", strlen("foo")));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_schema_set_type(schema, EVCPE_TYPE_OBJECT));

	TEST_ASSERT_NOT_NULL((obj = evcpe_obj_new(cls, NULL)));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_init(obj));

	TEST_ASSERT(0 != evcpe_obj_get(obj, "bar", strlen("bar"), &attr));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_get(obj, "foo", strlen("foo"), &attr));
	TEST_ASSERT_NOT_NULL(attr);

	TEST_ASSERT(0 != evcpe_attr_set(attr, "bar", strlen("bar")));
	TEST_ASSERT(0 != evcpe_attr_get(attr, &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_get_obj(attr, &child));
	TEST_ASSERT_NOT_NULL(child);
	evcpe_obj_free(obj);
}

static void test_multiple(void)
{
	struct evcpe_obj *child1, *child2;
	struct evcpe_attr_schema *schema;
	struct evcpe_attr *attr;
	const char *value;
	unsigned int index, len;

	TEST_ASSERT_EQUAL_INT(0, evcpe_class_add(cls, &schema));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_schema_set_name(schema, "foo", strlen("foo")));
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_schema_set_type(schema, EVCPE_TYPE_MULTIPLE));

	TEST_ASSERT_NOT_NULL((obj = evcpe_obj_new(cls, NULL)));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_init(obj));

	TEST_ASSERT(0 != evcpe_obj_get(obj, "bar", strlen("bar"), &attr));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_get(obj, "foo", strlen("foo"), &attr));
	TEST_ASSERT_NOT_NULL(attr);

	TEST_ASSERT(0 != evcpe_attr_set(attr, "bar", strlen("bar")));
	TEST_ASSERT(0 != evcpe_attr_get(attr, &value, &len));
//	TEST_ASSERT(0 != evcpe_attr_set_obj(attr, &child1));
	TEST_ASSERT(0 != evcpe_attr_get_obj(attr, &child1));
	TEST_ASSERT(0 != evcpe_attr_idx_obj(attr, 0, &child1));
	TEST_ASSERT(0 != evcpe_attr_idx_obj(attr, 1, &child2));

	child1 = NULL;
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_add_obj(attr, &child1, &index));
	TEST_ASSERT_NOT_NULL(child1);
	TEST_ASSERT_EQUAL_INT(0, index);

	child1 = NULL;
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_idx_obj(attr, index, &child1));
	TEST_ASSERT_NOT_NULL(child1);

	child2 = NULL;
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_add_obj(attr, &child2, &index));
	TEST_ASSERT_NOT_NULL(child2);
	TEST_ASSERT_EQUAL_INT(1, index);

	child2 = NULL;
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_idx_obj(attr, index, &child2));
	TEST_ASSERT_NOT_NULL(child2);

	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_del_obj(attr, 0));
	TEST_ASSERT(0 != evcpe_attr_idx_obj(attr, 0, &child1));
	child2 = NULL;
	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_idx_obj(attr, index, &child2));
	TEST_ASSERT_NOT_NULL(child2);

	TEST_ASSERT_EQUAL_INT(0, evcpe_attr_del_obj(attr, 1));
	TEST_ASSERT(0 != evcpe_attr_idx_obj(attr, 1, &child2));

	evcpe_obj_free(obj);
}

TestRef evcpe_obj_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("obj_test_init", test_init),
		new_TestFixture("obj_test_simple", test_simple),
		new_TestFixture("obj_test_obj", test_obj),
		new_TestFixture("obj_test_multiple", test_multiple),
	};
	EMB_UNIT_TESTCALLER(evcpe_obj_test_case, "obj_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&evcpe_obj_test_case;
}
