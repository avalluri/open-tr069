NULL = 
lib_LTLIBRARIES = libevcpe.la

libevcpe_la_SOURCES = \
	access_list.c \
	accessor.c accessor.h \
	add_object.c \
	add_object_xml.c \
	attr.c \
	attr_schema.c \
	attr_xml.c \
	class.c \
	class_xml.c \
	cookie.c \
	constraint.c \
	cpe.c cpe.h \
	data_xml.c \
	delete_object.c \
	dns_cache.c \
	event_list.c \
	fault.c \
	fault_xml.c \
	get_param_attrs.c \
	get_param_names.c \
	get_param_names_xml.c \
	get_param_values.c \
	get_param_values_xml.c \
	get_rpc_methods.c \
	get_rpc_methods_xml.c \
	handler.c \
	inform.c \
	inform_xml.c \
	log.c log.h \
	method.c \
	method_list.c \
	msg_xml.c \
	minixml.c \
	msg.c \
	obj.c obj.h \
	obj_list.c \
	obj_xml.c \
	param_attr_list.c \
	param_info_list.c \
	param_name_list.c \
	param_value_list.c \
	persister.c \
	repo.c \
	response.c xml_\
	session.c session.h \
	set_param_attr_list.c \
	set_param_attrs.c \
	set_param_value_list.c \
	set_param_values.c \
	set_param_values_xml.c \
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

bin_PROGRAMS = evcpe
evcpe_SOURCES = evcpe.c evcpe.h
evcpe_CPPFLAGS = \
	-I$(abs_top_srcdir)/src/compat \
	-I$(LIBEVENT_CFLAGS) \
	$(NULL)

evcpe_LDADD = $(LIBEVENT_LIBS) libevcpe.la -ldl
