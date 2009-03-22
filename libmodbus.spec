Summary: A Modbus library in C, which supports RTU communication over a serial line or a TCP link. 
Name: libmodbus
Version: 2.0.3
Release: 1
License: LGPL V3+
Packager: Some random Internet user
URL: https://launchpad.net/libmodbus/
Group: Applications/System          
Provides: libmodbus=2.0.3
Requires: ,/bin/sh

Source0: libmodbus-2.0.3.tar.bz2

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: autoconf, automake

%description
A Modbus library for Linux (and OSX) wrote in C and which supports 
RTU communication over a serial line or a TCP link. Clean and fast!
Supports controling an RTU and being an RTU.

%prep
%setup -q

autoreconf

%build
%configure 

make


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m755 $RPM_BUILD_ROOT/
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p -m755 $RPM_BUILD_ROOT/usr/share/libmodbus/
ls -lRh $RPM_BUILD_ROOT/


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root)
%attr(0755,root,root) %dir %{_libdir}
%attr(0755,root,root) %dir %{_libdir}/pkgconfig
%attr(0755,root,root) %dir %{_includedir}
%attr(0755,root,root) %dir %{_includedir}/modbus/
%dir %{_libdir}/libmodbus.so.2
%dir %{_libdir}/libmodbus.so
%attr(0755,root,root) %{_libdir}/libmodbus.so.2.0.3
%attr(0755,root,root) %{_libdir}/libmodbus.la
%attr(0644,root,root) %{_libdir}/pkgconfig/modbus.pc
%attr(0644,root,root) %{_includedir}/modbus/modbus.h
%doc AUTHORS ChangeLog INSTALL NEWS COPYING* README


%changelog
* Sun Mar 22 2009 Stéphane Raimbault <stephane.raimbault@gmail.com> - 2.0.3-1
- new upstream release

* Sun Aug 10 2008 Stéphane Raimbault <stephane.raimbault@gmail.com> - 2.0.2-1
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
