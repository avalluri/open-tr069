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

#include <mcheck.h>
#include <getopt.h>
#include <stdio.h>

#include <textui/TextOutputter.h>
#include <textui/TextUIRunner.h>
#include <embUnit/TestResult.h>
#include <embUnit/TestListener.h>

#include "log.h"
#include "test_suite.h"

#define TESTS_LOG_FILE "evcpe-tests.log"
static FILE* _fp = NULL;
static OutputterRef _txtOutputter = NULL;

static void _printHeader(OutputterRef self)
{
	Outputter_printHeader(_txtOutputter);
}

static void _printStartTest(OutputterRef self,TestRef test)
{
	fprintf(_fp, "+ Start of %s\n", Test_name(test));

	Outputter_printStartTest(_txtOutputter, test);
}

static void _printEndTest(OutputterRef self,TestRef test)
{
	fprintf(_fp, "+ End of %s\n", Test_name(test));

	Outputter_printEndTest(_txtOutputter, test);
}

static void _printSuccessful(OutputterRef self, TestRef test, int runCount)
{
	Outputter_printSuccessful(_txtOutputter, test, runCount);
}

static void _printFailure(OutputterRef self, TestRef test, char *msg, int line,
		char *file,int runCount)
{
	Outputter_printFailure(_txtOutputter, test, msg, line, file, runCount);
}

static void _printStatistics(OutputterRef self,TestResultRef result)
{
	Outputter_printStatistics(_txtOutputter, result);
}

static const OutputterImplement _outputterImpl = {
	(OutputterPrintHeaderFunction)		_printHeader,
	(OutputterPrintStartTestFunction)	_printStartTest,
	(OutputterPrintEndTestFunction)		_printEndTest,
	(OutputterPrintSuccessfulFunction)	_printSuccessful,
	(OutputterPrintFailureFunction)		_printFailure,
	(OutputterPrintStatisticsFunction)	_printStatistics,
};

Outputter _outputter = {
	(OutputterImplementRef)&_outputterImpl,
};

int main(int argc, char **argv)
{
	int c;
	enum evcpe_log_level level = EVCPE_LOG_INFO;
	OutputterRef out = NULL;

	while ((c = getopt(argc, argv, "vs")) != -1) {
		switch (c) {
		case 's':
			level ++;
			break;
		case 'v':
			if (level > EVCPE_LOG_TRACE)
				level --;
		}
	}

	mtrace();

	if (!(_fp = fopen(TESTS_LOG_FILE, "w+"))) {
		fprintf(stderr, "Failed to open log file: %s", TESTS_LOG_FILE);
		_fp = stdout;
	}

	if (level <= EVCPE_LOG_FATAL) {
		evcpe_add_logger("stderr", level, EVCPE_LOG_FATAL,NULL,
				evcpe_file_logger, _fp);
	}

	INFO("starting test");

	_txtOutputter = TextOutputter_outputter();

	if (_fp == stdout)
		out = _txtOutputter;
	else out = &_outputter;

	TextUIRunner_startWithOutputter(out);
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

	if (_fp != stdout) fclose(_fp);

	muntrace();

	return 0;
}
