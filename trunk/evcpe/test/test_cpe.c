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
