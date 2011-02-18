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
#include <stdio.h>

#include "msg_xml.h"
#include "fault.h"
#include "inform.h"
#include "get_rpc_methods.h"
#include "get_param_names.h"
#include "set_param_values.h"
#include "get_param_values.h"
#include "set_param_attrs.h"
#include "get_param_attrs.h"
#include "add_object.h"
#include "delete_object.h"

#include "test_suite.h"

static struct evbuffer *buffer;
static struct evcpe_msg *msg;

static void test_setup(void)
{
	buffer = evbuffer_new();
	msg = evcpe_msg_new();
}

static void test_teardown(void)
{
	evcpe_msg_free(msg);
	evbuffer_free(buffer);
}

static void test_init(void)
{
	TEST_ASSERT_NOT_NULL(buffer);
	TEST_ASSERT_NOT_NULL(msg);
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

static void test_inform_response(void)
{
	struct evcpe_inform_response *method;
	test_load("testfiles/inform_response.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_RESPONSE, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_INFORM, msg->method_type);
	method = msg->data;
	TEST_ASSERT_EQUAL_INT(1, method->max_envelopes);
}

static void test_get_rpc_methods(void)
{
	test_load("testfiles/get_rpc_methods.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(1, msg->no_more_requests);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_RPC_METHODS, msg->method_type);
}

static void test_get_rpc_methods_response(void)
{
	struct evcpe_get_rpc_methods_response *method;
	struct evcpe_method_list_item *item;
	int i;
	test_load("testfiles/get_rpc_methods_response.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_RESPONSE, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_RPC_METHODS, msg->method_type);
	method = msg->data;
	TEST_ASSERT(!TAILQ_EMPTY(&method->method_list.head));
	i = 0;
	TAILQ_FOREACH(item, &method->method_list.head, entry) {
		switch(i) {
		case 0:
			TEST_ASSERT_EQUAL_STRING("GetRPCMethods", item->name);
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING("GetParameterNames", item->name);
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING("GetParameterValues", item->name);
			break;
		case 3:
			TEST_ASSERT_EQUAL_STRING("SetParameterValues", item->name);
			break;
		case 4:
			TEST_ASSERT_EQUAL_STRING("SetParameterAttributes", item->name);
			break;
		case 5:
			TEST_ASSERT_EQUAL_STRING("GetParameterAttributes", item->name);
			break;
		case 6:
			TEST_ASSERT_EQUAL_STRING("AddObject", item->name);
			break;
		case 7:
			TEST_ASSERT_EQUAL_STRING("DeleteObject", item->name);
			break;
		}
		i++;
	}
}

static void test_set_param_values(void)
{
	struct evcpe_set_param_values *method;
	struct evcpe_set_param_value *param;
	int i;
	test_load("testfiles/set_parameter_values.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_SET_PARAMETER_VALUES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(evcpe_set_param_value_list_size(&method->parameter_list));
	i = 0;
	TAILQ_FOREACH(param, &method->parameter_list.head, entry) {
		switch(i) {
		case 0:
			TEST_ASSERT_EQUAL_STRING(".a.d", param->name);
			TEST_ASSERT(!strncmp("a new string", param->data, param->len));
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING(".a.b.a", param->name);
			TEST_ASSERT(!strncmp("54321", param->data, param->len));
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING(".a.b.b", param->name);
			TEST_ASSERT(!strncmp("ZmVkY2JhOTg3NjU0MzIxCg==", param->data, param->len));
			break;
		}
		i++;
	}
}

static void test_get_param_values(void)
{
	struct evcpe_get_param_values *method;
	struct evcpe_param_name *param;
	int i;
	test_load("testfiles/get_parameter_values.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_PARAMETER_VALUES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(0 != evcpe_param_name_list_size(&method->parameter_names));
	i = 0;
	TAILQ_FOREACH(param, &method->parameter_names.head, entry) {
		switch(i) {
		case 0:
			TEST_ASSERT_EQUAL_STRING(".a.d", param->name);
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING(".a.b.a", param->name);
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING(".a.b.b", param->name);
			break;
		case 3:
			TEST_ASSERT_EQUAL_STRING(".a.b.c", param->name);
			break;
		}
		i++;
	}
}

static void test_set_param_attrs(void)
{
	struct evcpe_set_param_attrs *method;
	struct evcpe_set_param_attr *param;
	struct evcpe_access_list_item *item;
	int i;
	test_load("testfiles/set_parameter_attributes.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_SET_PARAMETER_ATTRIBUTES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(!TAILQ_EMPTY(&method->parameter_list.head));
	i = 0;
	TAILQ_FOREACH(param, &method->parameter_list.head, entry) {
		switch(i) {
		case 0:
			TEST_ASSERT_EQUAL_STRING(".a.d", param->name);
			TEST_ASSERT_EQUAL_INT(1, param->notification_change);
			TEST_ASSERT_EQUAL_INT(2, param->notification);
			TEST_ASSERT_EQUAL_INT(1, param->access_list_change);
			TEST_ASSERT(!TAILQ_EMPTY(&param->access_list.head));
			item = TAILQ_FIRST(&param->access_list.head);
			TEST_ASSERT(!strcmp("Subscriber", item->entity));
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING(".a.b.a", param->name);
			TEST_ASSERT_EQUAL_INT(0, param->notification_change);
			TEST_ASSERT_EQUAL_INT(1, param->notification);
			TEST_ASSERT_EQUAL_INT(1, param->access_list_change);
			TEST_ASSERT(TAILQ_EMPTY(&param->access_list.head));
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING(".a.c.2.d", param->name);
			TEST_ASSERT_EQUAL_INT(1, param->notification_change);
			TEST_ASSERT_EQUAL_INT(2, param->notification);
			TEST_ASSERT_EQUAL_INT(0, param->access_list_change);
			TEST_ASSERT(TAILQ_EMPTY(&param->access_list.head));
			break;
		}
		i++;
	}
}

static void test_get_param_names(void)
{
	struct evcpe_get_param_names *method;
	test_load("testfiles/get_parameter_names.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	method = msg->data;
	TEST_ASSERT_EQUAL_STRING(".a.c.", method->parameter_path);
	TEST_ASSERT_EQUAL_INT(1, method->next_level);
}

static void test_get_param_names_simple(void)
{
	struct evcpe_get_param_names *method;
	test_load("testfiles/get_parameter_names_simple.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	method = msg->data;
	TEST_ASSERT_EQUAL_STRING(".a.d", method->parameter_path);
	TEST_ASSERT_EQUAL_INT(0, method->next_level);
}

static void test_get_param_names_all(void)
{
	struct evcpe_get_param_names *method;
	test_load("testfiles/get_parameter_names_all.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	method = msg->data;
	TEST_ASSERT_EQUAL_STRING("", method->parameter_path);
	TEST_ASSERT_EQUAL_INT(0, method->next_level);
}

static void test_get_param_attrs(void)
{
	struct evcpe_get_param_attrs *method;
	struct evcpe_param_name *param;
	int i;
	test_load("testfiles/get_parameter_attributes.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_PARAMETER_ATTRIBUTES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(0 != evcpe_param_name_list_size(&method->parameter_names));
	i = 0;
	TAILQ_FOREACH(param, &method->parameter_names.head, entry) {
		switch(i) {
		case 0:
			TEST_ASSERT_EQUAL_STRING(".a.d", param->name);
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING(".a.b.a", param->name);
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING(".a.c.2.d", param->name);
			break;
		}
		i++;
	}
}

static void test_add_object(void)
{
	struct evcpe_add_object *method;
	test_load("testfiles/add_object.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_ADD_OBJECT, msg->method_type);
	method = msg->data;
	TEST_ASSERT(!strcmp(".a.c.", method->object_name));
	TEST_ASSERT(!strcmp("Some Parameter Key", method->parameter_key));
}

static void test_delete_object(void)
{
	struct evcpe_delete_object *method;
	test_load("testfiles/delete_object.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_DELETE_OBJECT, msg->method_type);
	method = msg->data;
	TEST_ASSERT(!strcmp(".a.c.8.", method->object_name));
	TEST_ASSERT(!strcmp("Some Parameter Key", method->parameter_key));
}

static void test_fault(void)
{
	struct evcpe_fault *fault;
	test_load("testfiles/fault.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_FAULT, msg->type);
	fault = msg->data;
	TEST_ASSERT_EQUAL_INT(8002, fault->code);
	TEST_ASSERT_EQUAL_STRING("8002 - Internal Server error", fault->string);
}

TestRef evcpe_msg_xml_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("msg_xml_test_init", test_init),
		new_TestFixture("msg_xml_get_rpc_methods", test_get_rpc_methods),
		new_TestFixture("msg_xml_get_param_names", test_get_param_names),
		new_TestFixture("msg_xml_get_param_names_simple", test_get_param_names_simple),
		new_TestFixture("msg_xml_get_param_names_all", test_get_param_names_all),
		new_TestFixture("msg_xml_get_param_values", test_get_param_values),
		new_TestFixture("msg_xml_set_param_values", test_set_param_values),
		new_TestFixture("msg_xml_get_param_attrs", test_get_param_attrs),
		new_TestFixture("msg_xml_set_param_attrs", test_set_param_attrs),
		new_TestFixture("msg_xml_get_rpc_methods_response", test_get_rpc_methods_response),
		new_TestFixture("msg_xml_inform_response", test_inform_response),
		new_TestFixture("msg_xml_add_object", test_add_object),
		new_TestFixture("msg_xml_delete_object", test_delete_object),
		new_TestFixture("msg_xml_fault", test_fault),
	};
	EMB_UNIT_TESTCALLER(msg_xml_test_case, "msg_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&msg_xml_test_case;
}
