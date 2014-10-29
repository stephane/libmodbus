dnl ##############################################################################
dnl # AC_LIBMODBUS_CHECK_BUILD_DOC                                               #
dnl # Check whether to build documentation and install man-pages                 #
dnl ##############################################################################
AC_DEFUN([AC_LIBMODBUS_CHECK_BUILD_DOC], [{
    # Allow user to disable doc build
    AC_ARG_WITH([documentation], [AS_HELP_STRING([--without-documentation],
        [disable documentation build even if asciidoc and xmlto are present [default=no]])])

    if test "x$with_documentation" = "xno"; then
        ac_libmodbus_build_doc="no"
    else
        # Determine whether or not documentation should be built and installed.
        ac_libmodbus_build_doc="yes"
        # Check for asciidoc and xmlto and don't build the docs if these are not installed.
        AC_CHECK_PROG(ac_libmodbus_have_asciidoc, asciidoc, yes, no)
        AC_CHECK_PROG(ac_libmodbus_have_xmlto, xmlto, yes, no)
        if test "x$ac_libmodbus_have_asciidoc" = "xno" -o "x$ac_libmodbus_have_xmlto" = "xno"; then
           ac_libmodbus_build_doc="no"
        fi
    fi

    AC_MSG_CHECKING([whether to build documentation])
    AC_MSG_RESULT([$ac_libmodbus_build_doc])
    if test "x$ac_libmodbus_build_doc" = "xno"; then
        AC_MSG_WARN([The tools to build the documentation aren't installed])
    fi
    AM_CONDITIONAL(BUILD_DOC, test "x$ac_libmodbus_build_doc" = "xyes")
}])
