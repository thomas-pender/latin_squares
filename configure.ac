AC_PREREQ([2.71])
AC_INIT([latin_squares], [0.0])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/parallel_transversals.c])
AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])

AM_PROG_AR

LT_PREREQ([2.4.6])
LT_INIT([dlopen])

: ${CFLAGS="-O3"}
AC_PROG_CC

#-------------------------------------------------
# asynchronous execution
#-------------------------------------------------
AX_PTHREAD([],[AC_MSG_ERROR([no pthreads library found])])
CC=$PTHREAD_CC
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#-------------------------------------------------

#-------------------------------------------------
# Checks for typedefs, etc.
#-------------------------------------------------
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
#-------------------------------------------------

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

cat << EOF

============================================================

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

C compiler: '${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} ${LIBS}'

Compile with 'make'.

============================================================

EOF
