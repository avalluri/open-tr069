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

#include "msg.h"
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
static evcpe_msg *msg;

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

static void test_inform_response(void)
{
	evcpe_inform_response *method;
	const char* inform_response_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:InformResponse>\
          <cwmp:MaxEnvelopes>1</cwmp:MaxEnvelopes>\
        </cwmp:InformResponse>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", inform_response_rpc) > 0);
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_RESPONSE, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_INFORM, msg->method_type);
	method = msg->data;
	TEST_ASSERT_EQUAL_INT(1, method->max_envelopes);
}

static void test_get_rpc_methods(void)
{
	const char* get_rpc_methods_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
        <cwmp:NoMoreRequests>1</cwmp:NoMoreRequests>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:GetRPCMethods>\
        </cwmp:GetRPCMethods>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_rpc_methods_rpc) > 0);
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(1, msg->no_more_requests);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_RPC_METHODS, msg->method_type);
}

static void test_get_rpc_methods_response(void)
{
	evcpe_get_rpc_methods_response *method;
	tqueue_element* node = NULL;
	int i;
	evcpe_method_type_t test_data[] = {
			EVCPE_GET_RPC_METHODS,
			EVCPE_GET_PARAMETER_NAMES,
			EVCPE_GET_PARAMETER_VALUES,
			EVCPE_SET_PARAMETER_VALUES,
			EVCPE_SET_PARAMETER_ATTRIBUTES,
			EVCPE_GET_PARAMETER_ATTRIBUTES,
			EVCPE_ADD_OBJECT,
			EVCPE_DELETE_OBJECT
	};
	const char* get_rpc_methods_response_rpc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
		           xmlns:soap-enc=\"http://schemas.xmlsoap.org/soap/encoding/\"\
		           xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\"\
		           xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\
		           xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">\
	   <soap:Header>\
		<cwmp:ID soap:mustUnderstand=\"1\" xsi:type=\"xsd:string\">1234</cwmp:ID>\
	   </soap:Header>\
	   <soap:Body soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\
		 <cwmp:GetRPCMethodsResponse>\
			<MethodList xsi:type=\"soap-enc:Array\" soap-enc:arrayType=\"xsd:string[8]\">\
				<xsd:string xsi:type=\"xsd:string\">GetRPCMethods</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">GetParameterNames</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">GetParameterValues</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">SetParameterValues</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">SetParameterAttributes</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">GetParameterAttributes</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">AddObject</xsd:string>\
				<xsd:string xsi:type=\"xsd:string\">DeleteObject</xsd:string>\
			</MethodList>\
		  </cwmp:GetRPCMethodsResponse>\
	   </soap:Body>\
    </soap:Envelope>";


	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_rpc_methods_response_rpc) > 0);
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_RESPONSE, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_RPC_METHODS, msg->method_type);
	method = msg->data;
	TEST_ASSERT_NOT_NULL(method->method_list);
	TEST_ASSERT_EQUAL_INT(sizeof(test_data)/sizeof(*test_data),
			tqueue_size(method->method_list))
	i = 0;
	TQUEUE_FOREACH(node, method->method_list) {
		evcpe_method_type_t method = (evcpe_method_type_t)node->data;
		TEST_ASSERT(test_data[i] == method);
		i++;
	}
}

static void test_set_param_values(void)
{
	evcpe_set_param_values* method = NULL;
	evcpe_param_value *param = NULL;
	tqueue_element* node = NULL;
	int i = 0;
	char* test_data[][2] = {
			{ ".a.d", "a new string" },
			{ ".a.b.a", "54321" },
			{ ".a.b.b", "ZmVkY2JhOTg3NjU0MzIxCg==" }
	};
	const char* set_param_values_rcp = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:SetParameterValues>\
			<cwmp:ParameterList>\
			  <cwmp:ParameterValueStruct>\
				<cwmp:Name>.a.d</cwmp:Name>\
				<cwmp:Value>a new string</cwmp:Value>\
			  </cwmp:ParameterValueStruct>\
			  <cwmp:ParameterValueStruct>\
				<cwmp:Name>.a.b.a</cwmp:Name>\
				<cwmp:Value>54321</cwmp:Value>\
			  </cwmp:ParameterValueStruct>\
			  <cwmp:ParameterValueStruct>\
				<cwmp:Name>.a.b.b</cwmp:Name>\
				<cwmp:Value>ZmVkY2JhOTg3NjU0MzIxCg==</cwmp:Value>\
			  </cwmp:ParameterValueStruct>\
			</cwmp:ParameterList>\
			<cwmp:ParameterKey>Dummy_Key</cwmp:ParameterKey>\
        </cwmp:SetParameterValues>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", set_param_values_rcp) > 0);

	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_SET_PARAMETER_VALUES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(tqueue_size(method->parameter_list));

	TQUEUE_FOREACH(node, method->parameter_list) {
		param = (evcpe_param_value*)node->data;
		TEST_ASSERT_EQUAL_STRING(test_data[i][0], param->name);
		TEST_ASSERT(!strncmp(test_data[i][1], param->data, param->len));
		i++;
	}
}

static void test_get_param_values(void)
{
	evcpe_get_param_values *method;
	tqueue_element* node = NULL;
	char *test_data[] = { ".a.d", ".a.b.a", ".a.b.b", ".a.b.c" };
	int i = 0;
	const char* get_param_values_rpc = "<?xml version=\"1.0\"?>\
	<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:GetParameterValues>\
			<cwmp:ParameterNames>\
			  <xsd:string>.a.d</xsd:string>\
			  <xsd:string>.a.b.a</xsd:string>\
			  <xsd:string>.a.b.b</xsd:string>\
			  <xsd:string>.a.b.c</xsd:string>\
			</cwmp:ParameterNames>\
        </cwmp:GetParameterValues>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_param_values_rpc) > 0);

	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_PARAMETER_VALUES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(0 != tqueue_size(method->parameter_names));
	i = 0;
	TQUEUE_FOREACH(node, method->parameter_names) {
		char *param = (char*)node->data;
		TEST_ASSERT_EQUAL_STRING(test_data[i], param);
		i++;
	}
}

static void test_set_param_attrs(void)
{
	evcpe_set_param_attrs *method;
	evcpe_set_param_attr *param;
	tqueue_element* node = NULL;
	struct _test_data {
		const char* param_name;
		int notification_change;
		int notification;
		int access_list_change;
		const char* access_list[1];
		size_t access_list_size;
	}test_data[] = {
			{".a.d", 1, 2, 1, { "Subscriber" }, 1 },
			{".a.b.a", 0, 1, 1, {} , 0},
			{".a.c.2.d", 1, 2, 0, {}, 0}
	};
	int i = 0;
	const char* set_param_attrs_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:SetParameterAttributes>\
		<cwmp:ParameterList>\
		  <cwmp:SetParameterAttributesStruct>\
			<cwmp:Name>.a.d</cwmp:Name>\
			<cwmp:NotificationChange>1</cwmp:NotificationChange>\
			<cwmp:Notification>2</cwmp:Notification>\
			<cwmp:AccessListChange>1</cwmp:AccessListChange>\
			<cwmp:AccessList>\
			  <xsd:string>Subscriber</xsd:string>\
			</cwmp:AccessList>\
		  </cwmp:SetParameterAttributesStruct>\
		  <cwmp:SetParameterAttributesStruct>\
			<cwmp:Name>.a.b.a</cwmp:Name>\
			<cwmp:NotificationChange>0</cwmp:NotificationChange>\
			<cwmp:Notification>1</cwmp:Notification>\
			<cwmp:AccessListChange>1</cwmp:AccessListChange>\
			<cwmp:AccessList>\
			</cwmp:AccessList>\
		  </cwmp:SetParameterAttributesStruct>\
		  <cwmp:SetParameterAttributesStruct>\
			<cwmp:Name>.a.c.2.d</cwmp:Name>\
			<cwmp:NotificationChange>1</cwmp:NotificationChange>\
			<cwmp:Notification>2</cwmp:Notification>\
			<cwmp:AccessList>\
			</cwmp:AccessList>\
		  </cwmp:SetParameterAttributesStruct>\
		</cwmp:ParameterList>\
        </cwmp:SetParameterAttributes>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", set_param_attrs_rpc) > 0);

	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_SET_PARAMETER_ATTRIBUTES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(!tqueue_empty(method->parameter_list));

	TQUEUE_FOREACH(node, method->parameter_list) {
		int j = 0;
		tqueue_element* node1 = NULL;
		param = (evcpe_set_param_attr*)node->data;
		TEST_ASSERT_EQUAL_STRING(test_data[i].param_name, param->info->name);
		TEST_ASSERT_EQUAL_INT(test_data[i].notification_change,
				param->notification_change);
		TEST_ASSERT_EQUAL_INT(test_data[i].notification,
				param->info->notification);
		TEST_ASSERT_EQUAL_INT(test_data[i].access_list_change,
				param->access_list_change);
		TEST_ASSERT_EQUAL_INT(test_data[i].access_list_size,
				tqueue_size(param->info->access_list));

		TQUEUE_FOREACH(node1, param->info->access_list) {
			char* access = node1->data;
			TEST_ASSERT(!strcmp(test_data[i].access_list[j], access));
			j++;
		}
		i++;
	}
}

static void test_get_param_names(void)
{
	evcpe_get_param_names *method;
	const char* get_param_names_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
        <cwmp:NoMoreRequests>1</cwmp:NoMoreRequests>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:GetParameterNames>\
  	      <cwmp:ParameterPath>.a.c.</cwmp:ParameterPath>\
  	      <cwmp:NextLevel>1</cwmp:NextLevel>\
        </cwmp:GetParameterNames>\
      </soap:Body>\
    </soap:Envelope>";


	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_param_names_rpc) > 0);
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	method = msg->data;
	TEST_ASSERT_EQUAL_STRING(".a.c.", method->parameter_path);
	TEST_ASSERT_EQUAL_INT(1, method->next_level);
}

static void test_get_param_names_simple(void)
{
	evcpe_get_param_names *method;
	const char* get_param_names_simple_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
        <cwmp:NoMoreRequests>1</cwmp:NoMoreRequests>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:GetParameterNames>\
  	      <cwmp:ParameterPath>.a.d</cwmp:ParameterPath>\
  	      <cwmp:NextLevel>0</cwmp:NextLevel>\
        </cwmp:GetParameterNames>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_param_names_simple_rpc) > 0);
	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	method = msg->data;
	TEST_ASSERT_EQUAL_STRING(".a.d", method->parameter_path);
	TEST_ASSERT_EQUAL_INT(0, method->next_level);
}

static void test_get_param_names_all(void)
{
	evcpe_get_param_names *method;
	const char* get_param_names_all_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
        <cwmp:NoMoreRequests>1</cwmp:NoMoreRequests>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:GetParameterNames>\
  	      <cwmp:ParameterPath></cwmp:ParameterPath>\
  	      <cwmp:NextLevel>0</cwmp:NextLevel>\
        </cwmp:GetParameterNames>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_param_names_all_rpc) > 0);

	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	method = msg->data;
	TEST_ASSERT_EQUAL_STRING("", method->parameter_path);
	TEST_ASSERT_EQUAL_INT(0, method->next_level);
}

static void test_get_param_attrs(void)
{
	evcpe_get_param_attrs *method;
	char* param = NULL;
	tqueue_element* node = NULL;
	char* test_data[] = { ".a.d", ".a.b.a", ".a.c.2.d" };
	int i = 0;
	const char* get_param_attrs_rpc = "<?xml version=\"1.0\"?>\
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:GetParameterAttributes>\
  	      <cwmp:ParameterNames>\
  	        <xsd:string>.a.d</xsd:string>\
  	        <xsd:string>.a.b.a</xsd:string>\
  	        <xsd:string>.a.c.2.d</xsd:string>\
  	      </cwmp:ParameterNames>\
        </cwmp:GetParameterAttributes>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", get_param_attrs_rpc) > 0);

	TEST_ASSERT_EQUAL_INT(0, evcpe_msg_from_xml(msg, buffer));
	TEST_ASSERT_EQUAL_STRING("1234", msg->session);
	TEST_ASSERT_EQUAL_INT(EVCPE_MSG_REQUEST, msg->type);
	TEST_ASSERT_EQUAL_INT(EVCPE_GET_PARAMETER_ATTRIBUTES, msg->method_type);
	method = msg->data;
	TEST_ASSERT(0 != tqueue_size(method->parameter_names));
	i = 0;
	TQUEUE_FOREACH(node, method->parameter_names) {
		param = (char*)node->data;
		TEST_ASSERT_EQUAL_STRING(test_data[i], param);
		i++;
	}
}

static void test_add_object(void)
{
	evcpe_add_object *method;
	const char* add_object_rpc = "<?xml version=\"1.0\"?> \
	<soap:Envelope \
	    xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
	    soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" \
	    xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
	      <soap:Header>\
	        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
	      </soap:Header>\
	      <soap:Body>\
	        <cwmp:AddObject>\
	  	<cwmp:ObjectName>.a.c.</cwmp:ObjectName>\
	  	<cwmp:ParameterKey>Some Parameter Key</cwmp:ParameterKey>\
	        </cwmp:AddObject>\
	      </soap:Body>\
	  </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", add_object_rpc) > 0);

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

	evcpe_delete_object *method;
	const char* delete_object_rpc = "<?xml version=\"1.0\"?> \
    <soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
                   soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"\
                   xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0 \">\
      <soap:Header>\
        <cwmp:ID soap:mustUnderstand=\"1\">1234</cwmp:ID>\
      </soap:Header>\
      <soap:Body>\
        <cwmp:DeleteObject>\
  	      <cwmp:ObjectName>.a.c.8.</cwmp:ObjectName>\
  	      <cwmp:ParameterKey>Some Parameter Key</cwmp:ParameterKey>\
        </cwmp:DeleteObject>\
      </soap:Body>\
    </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", delete_object_rpc) > 0);
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
	evcpe_fault *fault;
	const char* fault_rpc = "<?xml version=\"1.0\"?>\
	<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"\
			       xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\
			       xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\
			       xmlns:cwmp=\"urn:dslforum-org:cwmp-1-0\">\
	   <soap:Body>\
	    <soap:Fault>\
	     <faultcode>soap:Server</faultcode>\
	     <faultstring>CWMP fault</faultstring>\
	     <detail>\
	      <cwmp:Fault>\
	       <cwmp:FaultCode>8002</cwmp:FaultCode>\
	       <cwmp:FaultString>8002 - Internal Server error</cwmp:FaultString>\
	      </cwmp:Fault>\
	     </detail>\
	    </soap:Fault>\
	   </soap:Body>\
	  </soap:Envelope>";

	TEST_ASSERT(evbuffer_add_printf(buffer, "%s", fault_rpc) > 0);
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
