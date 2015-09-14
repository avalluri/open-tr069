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

#ifndef evcpe_TEST_SUITE_H_
#define evcpe_TEST_SUITE_H_

#include <embUnit/embUnit.h>

TestRef evcpe_test_case(void);
TestRef evcpe_url_test_case(void);
TestRef evcpe_cookie_test_case(void);
TestRef evcpe_obj_test_case(void);
TestRef evcpe_msg_xml_test_case(void);
TestRef evcpe_model_xml_test_case(void);
TestRef evcpe_repo_test_case(void);

#endif /* evcpe_TEST_SUITE_H_ */
