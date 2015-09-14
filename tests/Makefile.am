noinst_PROGRAMS = test_suite

test_suite_SOURCES = \
	test_suite.c \
	test_obj.c \
	test_cpe.c \
	test_msg_xml.c \
	test_model_xml.c \
	test_repo.c \
	test_url.c \
	test_cookie.c

test_suite_CPPFLAGS = \
	-I$(abs_top_srcdir)/embunit/ \
	-I$(abs_top_srcdir)/src/ \
	-I$(abs_top_srcdir)/src/compat/

test_suite_LDFALGS = \
	-L$(abs_top_builddir)/embunit/lib/
	
test_suite_LDADD = \
	$(abs_top_builddir)/embunit/lib/libembUnit.a \
	$(abs_top_builddir)/embunit/lib/libtextui.a \
	$(abs_top_builddir)/src/libevcpe.la \
	-ldl $(LIBEVENT_LIBS)
