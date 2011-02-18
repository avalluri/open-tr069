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

#include "test_suite.h"

static struct evbuffer *buffer;
static struct evcpe_class *cls;
static struct evcpe_obj *obj;
static struct evcpe_repo *repo;
struct evcpe_inform *inform;

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
	evbuffer_drain(buffer, EVBUFFER_LENGTH(buffer));
	while ((len = evbuffer_read(buffer, fd, -1)));
	fclose(file);
}

static void test_tr098(void)
{
	struct evcpe_obj *temp;
	const char *value;
	unsigned int len;
	struct evcpe_attr *attr;
	struct evcpe_param_info *info;
	struct evcpe_param_info_list info_list;
	struct evcpe_param_attr *pattr;
	struct evcpe_param_attr_list pattr_list;
	struct evcpe_access_list_item *entity;
	unsigned int count;

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

	evcpe_param_info_list_init(&info_list);
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			"", &info_list, 1));
	TEST_ASSERT_NOT_NULL((info = TAILQ_FIRST(&info_list.head)));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.", info->name);
	TEST_ASSERT_EQUAL_INT(0, info->writable);
	evcpe_param_info_list_clear(&info_list);

	evcpe_param_info_list_init(&info_list);
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME, evcpe_repo_to_param_info_list(
			repo, ".DeviceInfo.SerialNumber.", &info_list, 0));
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_ARGUMENTS, evcpe_repo_to_param_info_list(
			repo, ".DeviceInfo.SerialNumber", &info_list, 1));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			".DeviceInfo.SerialNumber", &info_list, 0));
	TEST_ASSERT_NOT_NULL((info = TAILQ_FIRST(&info_list.head)));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SerialNumber", info->name);
	TEST_ASSERT_EQUAL_INT(0, info->writable);
	evcpe_param_info_list_clear(&info_list);

	evcpe_param_info_list_init(&info_list);
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			".DeviceInfo.", &info_list, 1));
	TEST_ASSERT(!TAILQ_EMPTY(&info_list.head));
	count = 0;
	TAILQ_FOREACH(info, &info_list.head, entry) {
		switch (count) {
		case 0:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.Manufacturer", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ManufacturerOUI", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ModelName", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 3:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.Description", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 4:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ProductClass", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 5:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SerialNumber", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 6:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.HardwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 7:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SoftwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 8:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ModemFirmwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 9:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.EnabledOptions", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 10:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.AdditionalHardwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 11:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.AdditionalSoftwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 12:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SpecVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 13:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ProvisioningCode", info->name);
			TEST_ASSERT_EQUAL_INT(1, info->writable);
			break;
		case 14:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.UpTime", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 15:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.FirstUseDate", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 16:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.DeviceLog", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 17:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFileNumberOfEntries", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 18:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.", info->name);
			TEST_ASSERT_EQUAL_INT(1, info->writable);
			break;
		default:
			TEST_FAIL("unexpected entry");
		}
		count ++;
	}
	evcpe_param_info_list_clear(&info_list);

	evcpe_param_info_list_init(&info_list);
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_info_list(repo,
			".DeviceInfo.", &info_list, 0));
	TEST_ASSERT(!TAILQ_EMPTY(&info_list.head));
	count = 0;
	TAILQ_FOREACH(info, &info_list.head, entry) {
		switch (count) {
		case 0:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 1:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.Manufacturer", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 2:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ManufacturerOUI", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 3:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ModelName", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 4:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.Description", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 5:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ProductClass", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 6:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SerialNumber", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 7:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.HardwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 8:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SoftwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 9:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ModemFirmwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 10:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.EnabledOptions", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 11:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.AdditionalHardwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 12:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.AdditionalSoftwareVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 13:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.SpecVersion", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 14:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.ProvisioningCode", info->name);
			TEST_ASSERT_EQUAL_INT(1, info->writable);
			break;
		case 15:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.UpTime", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 16:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.FirstUseDate", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 17:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.DeviceLog", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 18:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFileNumberOfEntries", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 19:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.", info->name);
			TEST_ASSERT_EQUAL_INT(1, info->writable);
			break;
		case 20:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.", info->name);
			TEST_ASSERT_EQUAL_INT(1, info->writable);
			break;
		case 21:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.Name", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 22:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.Version", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 23:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.Date", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 24:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.1.Description", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 25:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.2.", info->name);
			TEST_ASSERT_EQUAL_INT(1, info->writable);
			break;
		case 26:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.2.Name", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 27:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.2.Version", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 28:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.2.Date", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		case 29:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.VendorConfigFile.2.Description", info->name);
			TEST_ASSERT_EQUAL_INT(0, info->writable);
			break;
		default:
			TEST_FAIL("unexpected entry");
		}
		count ++;
	}
	evcpe_param_info_list_clear(&info_list);

	evcpe_param_attr_list_init(&pattr_list);
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME, evcpe_repo_to_param_attr_list(
			repo, "foobar", &pattr_list));
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME, evcpe_repo_to_param_attr_list(
			repo, "foobar.", &pattr_list));
	TEST_ASSERT_EQUAL_INT(EVCPE_CPE_INVALID_PARAM_NAME, evcpe_repo_to_param_attr_list(
			repo, ".DeviceSummary.", &pattr_list));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_attr_list(repo,
			".DeviceSummary", &pattr_list));
	TEST_ASSERT_NOT_NULL((pattr = TAILQ_FIRST(&pattr_list.head)));
	TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceSummary", pattr->name);
	TEST_ASSERT_EQUAL_INT(1, pattr->notification);
	TEST_ASSERT_NOT_NULL((entity = TAILQ_FIRST(&pattr->access_list.head)));
	TEST_ASSERT_EQUAL_STRING("Subscriber", entity->entity);
	evcpe_param_attr_list_clear(&pattr_list);

	evcpe_param_attr_list_init(&pattr_list);
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_param_attr_list(repo,
			".DeviceInfo.", &pattr_list));
	count = 0;
	TAILQ_FOREACH(pattr, &pattr_list.head, entry) {
		switch (count) {
		case 0:
			TEST_ASSERT_EQUAL_STRING("InternetGatewayDevice.DeviceInfo.Manufacturer", pattr->name);
			TEST_ASSERT_EQUAL_INT(0, pattr->notification);
			TEST_ASSERT(TAILQ_EMPTY(&pattr->access_list.head));
			break;
		}
		count ++;
		printf("%s\n", pattr->name);
	}
	evcpe_param_attr_list_clear(&pattr_list);

	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_to_inform(repo, inform));
	TEST_ASSERT_EQUAL_INT(0, evcpe_repo_del_event(repo, "0 BOOTSTRAP"));

	evbuffer_drain(buffer, EVBUFFER_LENGTH(buffer));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_to_xml(obj, buffer));
	evcpe_obj_free(obj);
//	printf("%.*s", EVBUFFER_LENGTH(buffer), EVBUFFER_DATA(buffer));
	TEST_ASSERT_NOT_NULL((obj = evcpe_obj_new(cls, NULL)));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_init(obj));
	TEST_ASSERT_EQUAL_INT(0, evcpe_obj_from_xml(obj, buffer));
//	TEST_ASSERT_NOT_NULL((repo = evcpe_repo_new(obj)));

	evcpe_repo_free(repo);
	evcpe_obj_free(obj);
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
