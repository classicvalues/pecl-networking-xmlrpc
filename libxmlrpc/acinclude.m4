# Local macros for autoconf

AC_DEFUN([XMLRPC_FUNCTION_CHECKS],[

# Standard XMLRPC list
AC_CHECK_FUNCS(strtoul strtoull)
])

AC_DEFUN([XMLRPC_HEADER_CHECKS],[
AC_CHECK_HEADERS(xmlparse.h xmltok.h strings.h)
])

AC_DEFUN([XMLRPC_TYPE_CHECKS],[

AC_REQUIRE([AC_C_INLINE])
AC_CHECK_SIZEOF(char, 1)

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
])
