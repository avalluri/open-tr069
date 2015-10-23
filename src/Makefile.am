NULL = 
lib_LTLIBRARIES = libevcpe-common.la

libevcpe_common_la_includedir = $(includedir)/evcpe
libevcpe_common_la_include_HEADERS = util.h log.h url.h plugin.h
libevcpe_common_la_SOURCES = util.c log.c url.c plugin.c
libevcpe_common_la_LDFLAGS = -rdynamic $(LDFLAGS)
libevcpe_common_la_LIBADD = -ldl

lib_LTLIBRARIES += libevcpe.la
libevcpe_la_SOURCES = \
	accessor.c accessor.h \
	add_object.c \
	attr.c \
	attr_schema.c \
	attr_xml.c \
	change_du_state.c change_du_state.h \
	class.c \
	class_xml.c \
	cookie.c \
	constraint.c \
	data.c data.h \
	data_xml.c data_xml.h \
	delete_object.c \
	download.c download.h \
	dns_cache.c \
	evcpe.c evcpe.h \
	fault.c \
	get_param_attrs.c \
	get_param_names.c \
	get_param_values.c \
	get_rpc_methods.c \
	handler.c handler.h \
	inform.c \
	log.c log.h \
	method.c \
	minixml.c \
	msg.c \
	obj.c obj.h \
	obj_xml.c \
	persister.c \
	plugin-priv.c plugin-priv.h \
	repo.c \
	response.c \
	session.c session.h \
	set_param_attrs.c \
	set_param_values.c \
	tqueue.c tqueue.h \
	type.c type.h \
	xml.c xml.h \
	xml_stack.c xml_stack.h \
	xmlns.c \
	$(NULL)


libevcpe_la_CPPFLAGS = \
	-I$(abs_top_srcdir)/src/compat \
	$(LIBEVENT_CFLAGS) \
	$(NULL)

libevcpe_la_LDFLAGS = -rdynamic $(LDFLAGS)
libevcpe_la_LIBDADD = $(LIBEVENT_LIBS) libevcpe-common.la -ldl

bin_PROGRAMS = evcpe
evcpe_SOURCES = main.c
evcpe_CFLAGS = \
	-I$(abs_top_srcdir)/src/compat \
	-I$(LIBEVENT_CFLAGS) \
	$(NULL)

evcpe_LDADD = $(LIBEVENT_LIBS) libevcpe.la libevcpe-common.la -ldl
