ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = src/libcliquer.la

src_libcliquer_la_SOURCES = $(top_srcdir)/cliquer-1.21/cliquer.h \
$(top_srcdir)/cliquer-1.21/set.h $(top_srcdir)/cliquer-1.21/graph.h \
$(top_srcdir)/cliquer-1.21/misc.h $(top_srcdir)/cliquer-1.21/reorder.h \
$(top_srcdir)/cliquer-1.21/cliquerconf.h $(top_srcdir)/cliquer-1.21/cl.c \
$(top_srcdir)/cliquer-1.21/cliquer.c $(top_srcdir)/cliquer-1.21/graph.c \
$(top_srcdir)/cliquer-1.21/reorder.c
src_libcliquer_la_CPPFLAGS = -I$(top_srcdir)/cliquer-1.21 -DENABLE_LONG_OPTIONS

noinst_PROGRAMS = src/transversals src/parallel_transversals src/mates src/moles \
src/parallel_mates src/find_transversal_depth

src_transversals_SOURCES = $(top_srcdir)/src/transversals.c
src_transversals_CPPFLAGS = -I$(top_srcdir)/src

src_mates_SOURCES = $(top_srcdir)/src/mates.c
src_mates_CPPFLAGS = -I$(top_srcdir)/src

src_parallel_transversals_SOURCES = $(top_srcdir)/src/parallel_transversals.c
src_parallel_transversals_CPPFLAGS = -I$(top_srcdir)/src

src_parallel_mates_SOURCES = $(top_srcdir)/src/parallel_mates.c
src_parallel_mates_CPPFLAGS = -I$(top_srcdir)/src

src_moles_SOURCES = $(top_srcdir)/src/moles.c
src_moles_CPPFLAGS = -I$(top_srcdir)/cliquer-1.21 -I$(top_srcdir)/src
src_moles_LDADD = $(top_builddir)/src/libcliquer.la

src_find_transversal_depth_SOURCES = $(top_srcdir)/src/find_transversal_depth.c
src_find_transversal_depth_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = $(top_srcdir)/test
