#! /bin/sh
do_symlink=
if ln -s autogen-symlink-test autogen-symlink-test-link > /dev/null 2>&1; then
    do_symlink=--symlink
    rm autogen-symlink-test-link
fi
if autoreconf --install $do_symlink --force; then
	echo
	echo "------------------------------------------------------"
	echo "Initialized build system. You can now run ./configure "
	echo "------------------------------------------------------"
	echo
else
	s="$?"
	echo
	echo "--------------------------"
	echo "Running autoreconf failed."
	echo "--------------------------"
	echo
	exit "$s"
fi
