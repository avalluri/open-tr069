NULL = 
lib_LTLIBRARIES = libevcpe.la

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
	handler.c \
	inform.c \
	log.c log.h \
	method.c \
	minixml.c \
	msg.c \
	obj.c obj.h \
	obj_xml.c \
	persister.c \
	plugin.c plugin.h \
	repo.c \
	response.c \
	session.c session.h \
	set_param_attrs.c \
	set_param_values.c \
	tqueue.c tqueue.h \
	type.c type.h \
	url.c url.h \
	util.c util.h \
	xml.c xml.h \
	xml_stack.c xml_stack.h \
	xmlns.c \
	$(NULL)


libevcpe_la_CPPFLAGS = \
	-I$(abs_top_srcdir)/src/compat \
	$(LIBEVENT_CFLAGS) \
	$(NULL)

libevcpe_la_LDFLAGS = -rdynamic $(LDFLAGS)
                  
libevcpe_la_LIBDADD = $(LIBEVENT_LIBS) -ldl

libevcpe_la_includedir = $(includedir)/evcpe
libevcpe_la_include_HEADERS = plugin.h

bin_PROGRAMS = evcpe
evcpe_SOURCES = main.c
evcpe_CPPFLAGS = \
	-I$(abs_top_srcdir)/src/compat \
	-I$(LIBEVENT_CFLAGS) \
	$(NULL)

evcpe_LDADD = $(LIBEVENT_LIBS) libevcpe.la -ldl
