## project metadata

NAME = evcpe
MAJOR_VER = 1
MINOR_VER = 0
PATCH_VER = 0

## toolchain

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip

## build flags

INCLUDES = -I. -Icompat -Iembunit
CFLAGS = -fPIC -Wall -g3 -O0 $(INCLUDES)
LDFLAGS = -ldl -levent -rdynamic
ARFLAGS = rcs

## prerequsites

EMBUNIT_LIBS = embunit/lib/libembUnit.a embunit/lib/libtextui.a

## artifacts

SRCS = session.c minixml.c log.c util.c handler.c class.c obj.c repo.c cpe.c dns_cache.c \
	event_list.c param_name_list.c param_value_list.c access_list.c \
	param_attr_list.c set_param_attr_list.c method_list.c \
	xml.c data_xml.c msg.c response.c xml_stack.c \
	xmlns.c inform.c get_rpc_methods.c get_param_values.c set_param_values.c \
	get_param_attrs.c set_param_attrs.c \
	add_object.c delete_object.c fault.c class_xml.c \
	obj_xml.c attr.c obj_list.c type.c attr_schema.c constraint.c url.c \
	msg_xml.c attr_xml.c method.c cookie.c inform_xml.c \
	get_rpc_methods_xml.c param_info_list.c get_param_names.c fault_xml.c \
	persister.c add_object_xml.c get_param_names_xml.c \
	get_param_values_xml.c set_param_value_list.c set_param_values_xml.c \
	accessor.c
OBJS = $(patsubst %.c,%.o,$(SRCS))

TEST_SRCS = test/test_suite.c test/test_obj.c test/test_cpe.c \
	test/test_msg_xml.c test/test_model_xml.c test/test_repo.c test/test_url.c \
	test/test_cookie.c

TEST_OBJS = $(patsubst %.c,%.o,$(TEST_SRCS))

EVCPE_SRCS = evcpe.c
EVCPE_OBJS = $(patsubst %.c,%.o,$(EVCPE_SRCS))

SHARED_LIB = lib$(NAME).so.$(MAJOR_VER).$(MINOR_VER).$(PATCH_VER)
SONAME = $(SHARED_LIB).$(MAJOR_VER)
LIB = lib$(NAME).a

TEST_SUITE = test_suite$(EXE)
EVCPE = evcpe$(EXE)

ARTIFACTS = $(LIB) $(SHARED_LIB) $(TEST_SUITE) $(EVCPE)

## targets

#.o.c:
#	$(CC) $(CFLAGS) -c $<

all: $(ARTIFACTS)

clean:
	rm -f *.o test/*.o $(ARTIFACTS)
	make RM="rm -f" -C embunit clean
	make RM="rm -f" -C embunit/textui clean
	
test: $(TEST_SUITE)
	rm -f $(TEST_SUITE).mtrace.log
	MALLOC_TRACE=$(TEST_SUITE).mtrace.log ./$(TEST_SUITE) -sss
	mtrace $(TEST_SUITE) $(TEST_SUITE).mtrace.log
	
run: $(EVCPE)
	rm -f $(EVCPE).mtrace.log
	MALLOC_TRACE=$(EVCPE).mtrace.log ./$(EVCPE) -m testfiles/tr098_model.xml \
		-d testfiles/tr098_data.xml -v bootstrap
	mtrace $(EVCPE) $(EVCPE).mtrace.log

$(SHARED_LIB): $(LIB)
	$(CC) -shared -Wl,-soname,$(SONAME) -o $@ $^

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(TEST_SUITE): $(TEST_OBJS) $(OBJS) $(EMBUNIT_LIBS) 
	$(CC) -o $@ $^ $(LDFLAGS)

$(EVCPE): $(EVCPE_OBJS) $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(EMBUNIT_LIBS):
	make -C embunit

.PHONY: clean all test run
