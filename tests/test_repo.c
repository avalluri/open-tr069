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

#include "class_xml.h"
#include "obj_xml.h"
#include "attr_xml.h"
#include "repo.h"
#include "fault.h"
#include "accessor.h"
#include "tqueue.h"

#include "test_suite.h"

static struct evbuffer *buffer;
static evcpe_class *cls;
static evcpe_obj *obj;
static evcpe_repo *repo;
evcpe_inform *inform;

static void test_setup(void)
{
	buffer = evbuffer_new();
	cls = evcpe_class_new(NULL);
	inform = evcpe_inform_new();
}

static void test_teardown(void)
{
	evcpe_inform_free(inform);
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
	evbuffer_drain(buffer, evbuffer_get_length(buffer));
	while ((len = evbuffer_read(buffer, fd, -1)));
	fclose(file);
}

static void test_tr098(void)
{
	evcpe_obj *temp;
	const char *value;
	unsigned int len;
	evcpe_attr *attr;
	evcpe_param_info *info;
	tqueue *info_list;
	evcpe_param_attr *pattr;
	tqueue *pattr_list;
	tqueue_element *node = NULL;
	unsigned int count;
	struct _DeviceInfo {
		const char* name;
		int writable;
	} device_info[] = {
			{ "DeviceCategory", 0 },
			{ "Manufacturer", 0 },
			{ "ManufacturerOUI", 0 },
			{ "ModelName", 0 },
			{ "ModelNumber", 0 },
			{ "Description", 0 },
			{ "ProductClass", 0 },
			{ "SerialNumber", 0 },
			{ "HardwareVersion", 0 },
			{ "SoftwareVersion", 0 },
			{ "ModemFirmwareVersion", 0 },
			{ "EnabledOptions", 0 },
			{ "AdditionalHardwareVersion", 0 },
			{ "AdditionalSoftwareVersion", 0 },
			{ "SpecVersion", 0 },
			{ "ProvisioningCode", 1 },
			{ "UpTime", 0 },
			{ "FirstUseDate", 0 },
			{ "DeviceLog", 0 },
			{ "VendorConfigFileNumberOfEntries", 0 },
			{ "SupportedDataModelNumberOfEntries", 0 },
			{ "ProcessorNumberOfEntries", 0 },
			{ "VendorLogFileNumberOfEntries", 0 },
			{ "LocationNumberOfEntries", 0 },
			{ "VendorConfigFile.", 1 },
			{ "MemoryStatus.", 0 },
			{ "ProcessStatus.", 0 },
			{ "TemperatureStatus.", 0 },
			{ "NetworkProperties.", 0 },
			{ "SupportedDataModel.", 0 },
			{ "Processor.", 0 },
			{ "VendorLogFile.", 0 },
			{ "ProxierInfo.", 0 },
			{ "Location.", 0 },
	},
	device_info_full[] = {
			{ "", 0 },
			{ "DeviceCategory", 0 },
			{ "Manufacturer", 0 },
			{ "ManufacturerOUI", 0 },
			{ "ModelName", 0 },
			{ "ModelNumber", 0 },
			{ "Description", 0 },
			{ "ProductClass", 0 },
			{ "SerialNumber", 0 },
			{ "HardwareVersion", 0 },
			{ "SoftwareVersion", 0 },
			{ "ModemFirmwareVersion", 0 },
			{ "EnabledOptions", 0 },
			{ "AdditionalHardwareVersion", 0 },
			{ "AdditionalSoftwareVersion", 0 },
			{ "SpecVersion", 0 },
			{ "ProvisioningCode", 1 },
			{ "UpTime", 0 },
			{ "FirstUseDate", 0 },
			{ "DeviceLog", 0 },
			{ "VendorConfigFileNumberOfEntries", 0 },
			{ "SupportedDataModelNumberOfEntries", 0 },
			{ "ProcessorNumberOfEntries", 0 },
			{ "VendorLogFileNumberOfEntries", 0 },
			{ "LocationNumberOfEntries", 0 },
			{ "VendorConfigFile.", 1 },
			{ "VendorConfigFile.1.", 1 },
			{ "VendorConfigFile.1.Alias", 1 },
			{ "VendorConfigFile.1.Name", 0 },
			{ "VendorConfigFile.1.Version", 0 },
			{ "VendorConfigFile.1.Date", 0 },
			{ "VendorConfigFile.1.Description", 0 },
			{ "VendorConfigFile.2.", 1 },
			{ "VendorConfigFile.2.Alias", 1 },
			{ "VendorConfigFile.2.Name", 0 },
			{ "VendorConfigFile.2.Version", 0 },
			{ "VendorConfigFile.2.Date", 0 },
			{ "VendorConfigFile.2.Description", 0 },
			{ "MemoryStatus.", 0 },
			{ "MemoryStatus.Total", 0 },
			{ "MemoryStatus.Free", 0 },
			{ "ProcessStatus.", 0 },
			{ "ProcessStatus.CPUUsage", 0 },
			{ "ProcessStatus.ProcessNumberOfEntries", 0 },
			{ "ProcessStatus.Process.", 0 },
			{ "TemperatureStatus.", 0 },
			{ "TemperatureStatus.TemperatureSensorNumberOfEntries", 0 },
			{ "TemperatureStatus.TemperatureSensor.", 0 },
			{ "NetworkProperties.", 0 },
			{ "NetworkProperties.MaxTCPWindowSize", 0 },
			{ "NetworkProperties.TCPImplementation", 0 },
			{ "SupportedDataModel.", 0 },
			{ "Processor.", 0 },
			{ "VendorLogFile.", 0 },
			{ "ProxierInfo.", 0 },
			{ "ProxierInfo.ManufacturerOUI", 0 },
			{ "ProxierInfo.ProductClass", 0 },
			{ "ProxierInfo.SerialNumber", 0 },
			{ "ProxierInfo.ProxyProtocol", 0 },
			{ "Location.", 0 }
	};


	int i;

	test_load("testfiles/tr098_model.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_class_from_xml(cls, buffer));
	TEST_ASSERT_NOT_NULL((obj = evcpe_obj_new(cls, NULL)));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_init(obj));
	test_load("testfiles/tr098_data.xml");
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_from_xml(obj, buffer));
	TEST_ASSERT_NOT_NULL((repo = evcpe_repo_new(obj)));

	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, "InternetGatewayDevice.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, ".", &temp));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.", temp->path);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".DeviceSummary.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".DeviceSummary", &value, &len));
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.DeviceSummary.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.DeviceSummary", &value, &len));

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".LANDeviceNumberOfEntries.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".LANDeviceNumberOfEntries", &value, &len));
	TEST_ASSERT_EQUAL_STRING("1", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.LANDeviceNumberOfEntries.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.LANDeviceNumberOfEntries", &value, &len));
	TEST_ASSERT_EQUAL_STRING("1", value);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".WANDeviceNumberOfEntries.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".WANDeviceNumberOfEntries", &value, &len));
	TEST_ASSERT_EQUAL_STRING("1", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.WANDeviceNumberOfEntries.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.WANDeviceNumberOfEntries", &value, &len));
	TEST_ASSERT_EQUAL_STRING("1", value);

	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, ".DeviceInfo", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, ".DeviceInfo.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice.DeviceInfo", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, "InternetGatewayDevice.DeviceInfo.", &temp));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.", temp->path);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".DeviceInfo.Manufacturer.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".DeviceInfo.Manufacturer", &value, &len));
	TEST_ASSERT_EQUAL_STRING("Foobar Inc.", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.Manufacturer.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.Manufacturer", &value, &len));
	TEST_ASSERT_EQUAL_STRING("Foobar Inc.", value);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".DeviceInfo.ManufacturerOUI.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".DeviceInfo.ManufacturerOUI", &value, &len));
	TEST_ASSERT_EQUAL_STRING("147AD2", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.ManufacturerOUI.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.ManufacturerOUI", &value, &len));
	TEST_ASSERT_EQUAL_STRING("147AD2", value);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".DeviceInfo.ProductClass.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".DeviceInfo.ProductClass", &value, &len));
	TEST_ASSERT_EQUAL_STRING("FooGate 2008", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.ProductClass.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.ProductClass", &value, &len));
	TEST_ASSERT_EQUAL_STRING("FooGate 2008", value);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".DeviceInfo.SerialNumber.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".DeviceInfo.SerialNumber", &value, &len));
	TEST_ASSERT_EQUAL_STRING("022882525200", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.SerialNumber.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.SerialNumber", &value, &len));
	TEST_ASSERT_EQUAL_STRING("022882525200", value);

	TEST_ASSERT(0 != evcpe_repo_get(repo, ".DeviceInfo.VendorConfigFileNumberOfEntries.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, ".DeviceInfo.VendorConfigFileNumberOfEntries", &value, &len));
	TEST_ASSERT_EQUAL_STRING("2", value);
	TEST_ASSERT(0 != evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.VendorConfigFileNumberOfEntries.", &value, &len));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get(repo, "InternetGatewayDevice.DeviceInfo.VendorConfigFileNumberOfEntries", &value, &len));
	TEST_ASSERT_EQUAL_STRING("2", value);

	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, ".DeviceInfo.VendorConfigFile", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, ".DeviceInfo.VendorConfigFile.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, ".DeviceInfo.VendorConfigFile.1", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, ".DeviceInfo.VendorConfigFile.1.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice.DeviceInfo.VendorConfigFile", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice.DeviceInfo.VendorConfigFile.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice.DeviceInfo.VendorConfigFile.1", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, "InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.", &temp));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.", temp->path);

	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice.ManagementServer", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, "InternetGatewayDevice.ManagementServer.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, ".ManagementServer", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, ".ManagementServer.", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_get(temp, "Authentication", strlen("Authentication"), &attr));
	TEST_ASSERT(attr->schema->extension != 0);

	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, "InternetGatewayDevice.Time", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, "InternetGatewayDevice.Time.", &temp));
	TEST_ASSERT(0 != evcpe_repo_get_obj(repo, ".Time", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_get_obj(repo, ".Time.", &temp));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_get(temp, "CurrentLocalTime", strlen("CurrentLocalTime"), &attr));
	TEST_ASSERT_NOT_NULL(attr->schema->getter);
	TEST_ASSERT_NOT_NULL(attr->schema->setter);
	TEST_ASSERT(attr->schema->getter == evcpe_get_curtime);
	TEST_ASSERT(attr->schema->setter == evcpe_set_curtime);

	info_list = evcpe_param_info_list_new();
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			"", info_list, 1));
	TEST_ASSERT_NOT_NULL((info = tqueue_first(info_list)->data));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.", info->name);
	TEST_ASSERT_EQUAL_INT(0, info->writable);
	tqueue_free(info_list);

	info_list = evcpe_param_info_list_new();
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME, evcpe_repo_to_param_info_list(
			repo, ".DeviceInfo.SerialNumber.", info_list, 0));
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_ARGUMENTS, evcpe_repo_to_param_info_list(
			repo, ".DeviceInfo.SerialNumber", info_list, 1));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			".DeviceInfo.SerialNumber", info_list, 0));
	TEST_ASSERT_NOT_NULL((info = tqueue_first(info_list)->data));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SerialNumber", info->name);
	TEST_ASSERT_EQUAL_INT(0, info->writable);
	tqueue_free(info_list);

	info_list = evcpe_param_info_list_new();
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			".DeviceInfo.", info_list, 1));
	TEST_ASSERT(tqueue_size(info_list) ==
			sizeof(device_info)/sizeof(*device_info));
	i = 0;
	TQUEUE_FOREACH(node, info_list) {
		char param[257];
		evcpe_param_info* info = node->data;
		snprintf(param, 256, "InternetGatewayDevice.DeviceInfo.%s", device_info[i].name);

		TEST_ASSERT_EQUAL_STRING(param, info->name);
		TEST_ASSERT_EQUAL_INT(device_info[i].writable, info->writable);
		i++;
	}
	tqueue_free(info_list);

	info_list = evcpe_param_info_list_new();
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			".DeviceInfo.", info_list, 0));
	TEST_ASSERT(tqueue_size(info_list) ==
			sizeof(device_info_full)/sizeof(*device_info_full));
	i = 0;
	TQUEUE_FOREACH(node, info_list) {
		char param[257];
		evcpe_param_info* info = node->data;
		snprintf(param, 256, "InternetGatewayDevice.DeviceInfo.%s",
				device_info_full[i].name);
		TEST_ASSERT_EQUAL_STRING(param, info->name);
		TEST_ASSERT_EQUAL_INT(device_info_full[i].writable, info->writable);
		i++;
	}
	tqueue_free(info_list);

	pattr_list = tqueue_new(NULL, (tqueue_free_func_t)evcpe_param_attr_free);
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME,
			evcpe_repo_to_param_attr_list(repo, "foobar", pattr_list));
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME,
			evcpe_repo_to_param_attr_list(repo, "foobar.", pattr_list));
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME,
			evcpe_repo_to_param_attr_list(repo, ".DeviceSummary.", pattr_list));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_attr_list(repo,
			".DeviceSummary", pattr_list));
	TEST_ASSERT_NOT_NULL((pattr = (evcpe_param_attr*)tqueue_first(pattr_list)->data));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceSummary", pattr->name);
	TEST_ASSERT_EQUAL_INT(EVCPE_NOTIFICATION_OFF, pattr->notification);
	TEST_ASSERT_NOT_NULL((node = tqueue_first(pattr->access_list)));
	TEST_ASSERT_EQUAL_STRING("Subscriber", (char*)node->data);
	tqueue_free(pattr_list);

	pattr_list = tqueue_new(NULL, (tqueue_free_func_t)evcpe_param_attr_free);
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_attr_list(repo,
			".DeviceInfo.", pattr_list));
	count = 0;
	TQUEUE_FOREACH(node, pattr_list) {
		pattr = (evcpe_param_attr*)node->data;
		switch (count) {
		case 0:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.DeviceCategory", pattr->name);
			TEST_ASSERT_EQUAL_INT(EVCPE_NOTIFICATION_OFF, pattr->notification);
			TEST_ASSERT(tqueue_empty(pattr->access_list));
			break;
		}
		count ++;
	}
	tqueue_free(pattr_list);

	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_inform(repo, inform));

	evbuffer_drain(buffer, evbuffer_get_length(buffer));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_to_xml(obj, buffer));
	evcpe_obj_free(obj);

	TEST_ASSERT_NOT_NULL((obj = evcpe_obj_new(cls, NULL)));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_init(obj));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_from_xml(obj, buffer));
	evcpe_obj_free(obj);

	evcpe_repo_free(repo);
}

TestRef evcpe_repo_test_case(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("repo_test_init", test_init),
		new_TestFixture("repo_tr098", test_tr098),
	};
	EMB_UNIT_TESTCALLER(repo_xml_test_case, "repo_xml_test_case", test_setup, test_teardown, fixtures);

	return (TestRef)&repo_xml_test_case;
}
