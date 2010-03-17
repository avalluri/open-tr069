// $Id$

#include <mcheck.h>
#include <getopt.h>

#include <textui/TextOutputter.h>
#include <textui/TextUIRunner.h>

#include "log.h"
#include "test_suite.h"

int main(int argc, char **argv)
{
	int c;
	enum evcpe_log_level level;

	level = EVCPE_LOG_INFO;
	while ((c = getopt(argc, argv, "vs")) != -1)
		switch (c) {
		case 's':
			level ++;
			break;
		case 'v':
			if (level > EVCPE_LOG_TRACE)
				level --;
		}

	mtrace();

	if (level <= EVCPE_LOG_FATAL)
		evcpe_add_logger("stderr", level, EVCPE_LOG_FATAL,
				NULL, evcpe_file_logger, stdout);

	evcpe_info(__func__, "starting test");

	TextUIRunner_setOutputter(TextOutputter_outputter());
	TextUIRunner_start();
	TextUIRunner_runTest(evcpe_msg_xml_test_case());
	TextUIRunner_runTest(evcpe_obj_test_case());
	TextUIRunner_runTest(evcpe_test_case());
	TextUIRunner_runTest(evcpe_model_xml_test_case());
	TextUIRunner_runTest(evcpe_url_test_case());
	TextUIRunner_runTest(evcpe_cookie_test_case());
	TextUIRunner_runTest(evcpe_repo_test_case());
	TextUIRunner_end();

	evcpe_remove_logger("stdout");
	evcpe_remove_logger("stderr");

	muntrace();

	return 0;
}
