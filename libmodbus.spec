Summary: A Modbus library written in C
Name: libmodbus
Version: 2.9.3
Release: 1%{?dist%}
License: LGPL V3+
Packager: Stéphane Raimbault
URL: http://www.libmodbus.org
Group: Applications/System
Provides: libmodbus=2.9.3
Requires: ,/bin/sh

Source0: libmodbus-2.9.3.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: autoconf, automake

%description
The library is written in C and designed to run on Linux, Mac OS X, FreeBSD and
QNX and Windows.

%prep
%setup -q

autoreconf

%build
%configure

make %{?_smp_mflags}


%install
rm -rf %{buildroot}
mkdir -p -m755 %{buildroot}/
make install DESTDIR=%{buildroot}
mkdir -p -m755 %{buildroot}/usr/share/libmodbus/
ls -lRh %{buildroot}/


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root)
%attr(0755,root,root) %dir %{_libdir}
%attr(0755,root,root) %dir %{_libdir}/pkgconfig
%attr(0755,root,root) %dir %{_includedir}
%attr(0755,root,root) %dir %{_includedir}/modbus/
%dir %{_libdir}/libmodbus.so.4
%dir %{_libdir}/libmodbus.so
%attr(0755,root,root) %{_libdir}/libmodbus.so.4.0.0
%attr(0755,root,root) %{_libdir}/libmodbus.la
%attr(0644,root,root) %{_libdir}/pkgconfig/libmodbus.pc
%attr(0644,root,root) %{_includedir}/modbus/modbus.h
%attr(0644,root,root) %{_includedir}/modbus/modbus-rtu.h
%attr(0644,root,root) %{_includedir}/modbus/modbus-tcp.h
%attr(0644,root,root) %{_includedir}/modbus/modbus-version.h
%doc AUTHORS MIGRATION NEWS COPYING* README.rst


%changelog
* Mon Jan 10 2011 Stéphane Raimbault <stephane.raimbault@gmail.com> - 2.9.3-1
- new upstream release

* Mon Oct 5 2010 Stéphane Raimbault <stephane.raimbault@gmail.com> - 2.9.2-1
- new upstream release

* Fri Jul 2 2008 Stéphane Raimbault <stephane.raimbault@gmail.com> - 2.0.1-1
- new upstream release

* Fri May 2 2008 Stéphane Raimbault <stephane.raimbault@gmail.com> - 2.0.0-1
- integrate extern_for_cpp in upstream.
- update the license to version LGPL v3.

* Tue Apr 30 2008 Todd Denniston <Todd.Denniston@ssa.crane.navy.mil> - 1.9.0-2
- get the license corrected in the spec file.
- add a URL for where to find libmodbus.
- tweak the summary and description.

* Tue Apr 29 2008 Todd Denniston <Todd.Denniston@ssa.crane.navy.mil> - 1.9.0-1
- upgrade to latest upstream (pre-release)
- port extern_for_cpp patch to 1.9.0

* Tue Apr 29 2008 Todd Denniston <Todd.Denniston@ssa.crane.navy.mil> - 1.2.4-2_tad
- add a patch to allow compiling with c++ code.

* Mon Apr 28 2008 Todd Denniston <Todd.Denniston@ssa.crane.navy.mil> - 1.2.4-1_tad
- build spec file.
- include patch for controling error-treat.
