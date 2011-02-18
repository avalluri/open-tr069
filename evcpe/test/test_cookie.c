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

#include "cookie.h"

static struct evcpe_cookies cookies;
static struct evcpe_cookie *cookie;

static void test_setup(void)
{
	RB_INIT(&cookies);
}

static void test_teardown(void)
{
	evcpe_cookies_clear(&cookies);
}

static void test_init(void)
{
	TEST_ASSERT(RB_EMPTY(&cookies));
}

static void test_set(void)
{
	TEST_ASSERT_NULL(evcpe_cookies_find(&cookies, "foo"));
	TEST_ASSERT(0 != evcpe_cookies_set(&cookies, NULL, NULL));
	TEST_ASSERT(0 != evcpe_cookies_set(&cookies, "foo", NULL));

	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set(&cookies, "foo", "bar"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "foo")));
	TEST_ASSERT_EQUAL_STRING("bar", cookie->value);

	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set(&cookies, "foo", "bar1"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "foo")));
	TEST_ASSERT_EQUAL_STRING("bar1", cookie->value);
}

static void test_set_from_header1(void)
{
	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set_from_header(&cookies,
			"MACID=000000000000 FileIndex=1 FileNumber=0"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "MACID")));
	TEST_ASSERT_EQUAL_STRING("000000000000 FileIndex=1 FileNumber=0", cookie->value);
}

static void test_set_from_header2(void)
{
	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set_from_header(&cookies,
			"MACID=000000000000 FileIndex=1 FileNumber=0; path=/"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "MACID")));
	TEST_ASSERT_EQUAL_STRING("000000000000 FileIndex=1 FileNumber=0", cookie->value);
}

static void test_set_from_header3(void)
{
	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set_from_header(&cookies,
			"MACID=000000000000; FileIndex=1; FileNumber=0"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "MACID")));
	TEST_ASSERT_EQUAL_STRING("000000000000", cookie->value);
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "FileIndex")));
	TEST_ASSERT_EQUAL_STRING("1", cookie->value);
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "FileNumber")));
	TEST_ASSERT_EQUAL_STRING("0", cookie->value);
}

static void test_set_from_header4(void)
{
	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set_from_header(&cookies,
			"MACID=000000000000; FileIndex=1; FileNumber=0; path=/"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "MACID")));
	TEST_ASSERT_EQUAL_STRING("000000000000", cookie->value);
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "FileIndex")));
	TEST_ASSERT_EQUAL_STRING("1", cookie->value);
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "FileNumber")));
	TEST_ASSERT_EQUAL_STRING("0", cookie->value);
}

static void test_set_from_header5(void)
{
	TEST_ASSERT_EQUAL_INT(0, evcpe_cookies_set_from_header(&cookies,
			"PREF=ID=20b72588e817329b:TM=1226850833:LM=1226930571:GM=1:S=adBNVg8yYKbAMnLg; expires=Sun, 17-Jan-2038; path=/; domain=.google.com"));
	TEST_ASSERT_NOT_NULL((cookie = evcpe_cookies_find(&cookies, "PREF")));
	TEST_ASSERT_EQUAL_STRING("ID=20b72588e817329b:TM=1226850833:LM=1226930571:GM=1:S=adBNVg8yYKbAMnLg", cookie->value);
}

TestRef evcpe_cookie_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("cookie_test_init", test_init),
		new_TestFixture("cookie_test_set", test_set),
		new_TestFixture("cookie_test_set_from_header1", test_set_from_header1),
		new_TestFixture("cookie_test_set_from_header2", test_set_from_header2),
		new_TestFixture("cookie_test_set_from_header3", test_set_from_header3),
		new_TestFixture("cookie_test_set_from_header4", test_set_from_header4),
		new_TestFixture("cookie_test_set_from_header5", test_set_from_header5),
	};
	EMB_UNIT_TESTCALLER(evcpe_cookie_test_case, "cookie_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&evcpe_cookie_test_case;
}
