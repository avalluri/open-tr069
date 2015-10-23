SUBDIRS = src src/plugins

if ENABLE_TESTS
SUBDIRS += embunit tests
endif

ACLOCAL_AMFLAGS = -I m4

tr098dir = $(datarootdir)/evcpe
tr098_DATA = data/tr098_data.xml	data/tr098_model.xml

EXTRA_DIST = $(tr098_DATA)
