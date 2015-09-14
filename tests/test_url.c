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

#include "url.h"

static struct evcpe_url *url;

static void test_setup(void)
{
	url = evcpe_url_new();
}

static void test_teardown(void)
{
	evcpe_url_free(url);
}

static void test_init(void)
{
	TEST_ASSERT_NOT_NULL(url);
}

static void test_invalid(void)
{
	evcpe_url_reset(url);
	TEST_ASSERT(0 != evcpe_url_from_str(url, ""));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "0"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "1"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "123"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "a"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "foobar"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "http"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "http://"));
	TEST_ASSERT(0 != evcpe_url_from_str(url, "http://foo:bar:com"));
}

static void test_http(void)
{
	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "http://www.google.com"));
	TEST_ASSERT_EQUAL_STRING("http", url->protocol);
	TEST_ASSERT_NULL(url->username);
	TEST_ASSERT_NULL(url->password);
	TEST_ASSERT_EQUAL_STRING("www.google.com", url->host);
	TEST_ASSERT_EQUAL_INT(80, url->port);
	TEST_ASSERT_NULL(url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "http://www.google.com:8080"));
	TEST_ASSERT_EQUAL_STRING("http", url->protocol);
	TEST_ASSERT_NULL(url->username);
	TEST_ASSERT_NULL(url->password);
	TEST_ASSERT_EQUAL_STRING("www.google.com", url->host);
	TEST_ASSERT_EQUAL_INT(8080, url->port);
	TEST_ASSERT_NULL(url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "https://www.google.com"));
	TEST_ASSERT_EQUAL_STRING("https", url->protocol);
	TEST_ASSERT_NULL(url->username);
	TEST_ASSERT_NULL(url->password);
	TEST_ASSERT_EQUAL_STRING("www.google.com", url->host);
	TEST_ASSERT_EQUAL_INT(443, url->port);
	TEST_ASSERT_NULL(url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "https://foo@foobar.com"));
	TEST_ASSERT_EQUAL_STRING("https", url->protocol);
	TEST_ASSERT_EQUAL_STRING("foo", url->username);
	TEST_ASSERT_NULL(url->password);
	TEST_ASSERT_EQUAL_STRING("foobar.com", url->host);
	TEST_ASSERT_EQUAL_INT(443, url->port);
	TEST_ASSERT_NULL(url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "http://foo:bar@foobar.com:8080"));
	TEST_ASSERT_EQUAL_STRING("http", url->protocol);
	TEST_ASSERT_EQUAL_STRING("foo", url->username);
	TEST_ASSERT_EQUAL_STRING("bar", url->password);
	TEST_ASSERT_EQUAL_STRING("foobar.com", url->host);
	TEST_ASSERT_EQUAL_INT(8080, url->port);
	TEST_ASSERT_NULL(url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "http://www.google.com/search"));
	TEST_ASSERT_EQUAL_STRING("http", url->protocol);
	TEST_ASSERT_NULL(url->username);
	TEST_ASSERT_NULL(url->password);
	TEST_ASSERT_EQUAL_STRING("www.google.com", url->host);
	TEST_ASSERT_EQUAL_INT(80, url->port);
	TEST_ASSERT_EQUAL_STRING("/search", url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "https://foo@foobar.com/cgi-bin"));
	TEST_ASSERT_EQUAL_STRING("https", url->protocol);
	TEST_ASSERT_EQUAL_STRING("foo", url->username);
	TEST_ASSERT_NULL(url->password);
	TEST_ASSERT_EQUAL_STRING("foobar.com", url->host);
	TEST_ASSERT_EQUAL_INT(443, url->port);
	TEST_ASSERT_EQUAL_STRING("/cgi-bin", url->uri);

	TEST_ASSERT_EQUAL_INT(0, evcpe_url_from_str(url, "http://foo:bar@foobar.com:8080/cgi-bin/query.cgi"));
	TEST_ASSERT_EQUAL_STRING("http", url->protocol);
	TEST_ASSERT_EQUAL_STRING("foo", url->username);
	TEST_ASSERT_EQUAL_STRING("bar", url->password);
	TEST_ASSERT_EQUAL_STRING("foobar.com", url->host);
	TEST_ASSERT_EQUAL_INT(8080, url->port);
	TEST_ASSERT_EQUAL_STRING("/cgi-bin/query.cgi", url->uri);
}

TestRef evcpe_url_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("url_test_init", test_init),
		new_TestFixture("url_test_invalid", test_invalid),
		new_TestFixture("url_test_http", test_http),
	};
	EMB_UNIT_TESTCALLER(evcpe_url_test_case, "url_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&evcpe_url_test_case;
}
