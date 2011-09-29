dnl ##############################################################################
dnl # AC_LIBMODBUS_CHECK_DOC_BUILD                                               #
dnl # Check whether to build documentation and install man-pages                 #
dnl ##############################################################################
AC_DEFUN([AC_LIBMODBUS_CHECK_DOC_BUILD], [{
    # Allow user to disable doc build
    AC_ARG_WITH([documentation], [AS_HELP_STRING([--without-documentation],
        [disable documentation build even if asciidoc and xmlto are present [default=no]])])

    if test "x$with_documentation" = "xno"; then
        ac_libmodbus_build_doc="no"
        ac_libmodbus_install_man="no"
    else
        # Determine whether or not documentation should be built and installed.
        ac_libmodbus_build_doc="yes"
        ac_libmodbus_install_man="yes"
        # Check for asciidoc and xmlto and don't build the docs if these are not installed.
        AC_CHECK_PROG(ac_libmodbus_have_asciidoc, asciidoc, yes, no)
        AC_CHECK_PROG(ac_libmodbus_have_xmlto, xmlto, yes, no)
        if test "x$ac_libmodbus_have_asciidoc" = "xno" -o "x$ac_libmodbus_have_xmlto" = "xno"; then
            ac_libmodbus_build_doc="no"
            # Tarballs built with 'make dist' ship with prebuilt documentation.
            if ! test -f doc/libmodbus.7; then
                ac_libmodbus_install_man="no"
                AC_MSG_WARN([You are building an unreleased version of libmodbus and asciidoc or xmlto are not installed.])
                AC_MSG_WARN([Documentation will not be built and manual pages will not be installed.])
            fi
        fi

        # Do not install man pages if on mingw
        if test "x$ac_libmodbus_on_mingw32" = "xyes"; then
            ac_libmodbus_install_man="no"
        fi
    fi

    AC_MSG_CHECKING([whether to build documentation])
    AC_MSG_RESULT([$ac_libmodbus_build_doc])

    AC_MSG_CHECKING([whether to install manpages])
    AC_MSG_RESULT([$ac_libmodbus_install_man])

    AM_CONDITIONAL(BUILD_DOC, test "x$ac_libmodbus_build_doc" = "xyes")
    AM_CONDITIONAL(INSTALL_MAN, test "x$ac_libmodbus_install_man" = "xyes")
}])
