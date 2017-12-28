TARGET = synmodbus
TEMPLATE = aux

MAKEFILE = Makefile.libsynmodbus

configure.target = $$PWD/Makefile
configure.commands = $$PWD/configure --prefix=$$INSTALL_PREFIX/
configure.depend_command = $$PWD/autogen.sh

all.commands = make all
all.depends = configure
all.CONFIG = phony

install.commands = make install
install.depends = all

QMAKE_DISTCLEAN += Makefile
QMAKE_EXTRA_TARGETS''= configure all install
