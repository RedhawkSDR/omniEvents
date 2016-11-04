%if 0%{?fedora} > 14 || 0%{?rhel} > 6
%global with_systemd 1
%endif

Summary: CORBA Event Service for omniORB
Name:    omniEvents
Version: 2.7.1
Release: 1%{?dist}
License: LGPL
Group:   System/Libraries
Source0: %{name}-%{version}.tar.gz
Source1: omniEvents.service
URL:     http://www.omnievents.org/
Buildroot:      %{_tmppath}/%{name}-%{version}-root
BuildRequires:  omniORB-devel glibc-devel boost-devel
%if 0%{?with_systemd}
%{?systemd_requires}
BuildRequires: systemd
%endif

%define lib_name %{?mklibname:%mklibname %{name} 2}%{!?mklibname:lib%{name}2}

%description
%{name} enables CORBA applications to communicate through asynchronous
broadcast channels rather than direct method calls.
It is a small, efficient implementation of the Object Management Group's
Event Service specification designed to work with omniORB.

%package -n %{lib_name}
Summary: CORBA Event Service for omniORB
Group:   System/Libraries
Prereq:  /sbin/ldconfig
Provides:       libomnievents = %{version}-%{release} %{name} = %{version}-%{release}

%description -n %{lib_name}
%{name} enables CORBA applications to communicate through asynchronous
broadcast channels rather than direct method calls.
It is a small, efficient implementation of the Object Management Group's
Event Service specification designed to work with omniORB.

%package server
Summary:   CORBA Event Service daemon
Group:     Development/C++
Prereq: omniORB-servers
Requires: %{lib_name} = %{version}-%{release}

%description server
The CORBA Event Service daemon as a standalone executable.

%package bootscripts
Summary:   Utility programs
Group:     Development/C++

Prereq:         /sbin/service /sbin/chkconfig
Prereq:    lsb >= 4.0
Requires:  %{name}-server = %{version}-%{release} %{name}-utils = %{version}-%{release}
Requires:  omniORB-utils

%description bootscripts
Automatic starting of the %{name} CORBA Event Service.

%package utils
Summary:   Utility programs
Group:     Development/C++
Requires:  %{lib_name} = %{version}-%{release}

%description utils
%{name} utility programs which may be useful at runtime.

%package -n %{lib_name}-devel
Summary: Header files and libraries needed for %{name} development
Group:          Development/C++
Requires:       %{lib_name} = %{version}-%{release}
Provides:       libomnievents-devel = %{version}-%{release} %{name}-devel = %{version}-%{release}
Conflicts:      libomnievents-devel > %{version}-%{release} libomnievents-devel < %{version}-%{release}

%description -n %{lib_name}-devel
The header files and libraries needed for developing programs using %{name}.

%package doc
Summary: Documentation and examples for %{name}
Group:          Development/C++

%description doc
Developer documentation and examples.

%prep
%setup -n %{name}-%{version}

%{?configure:%configure}%{!?configure:CPPFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix} --libdir=%{_libdir}} --enable-unloadable-stubs

%build
%{?make:%make}%{!?make:make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS"} all
%{?make:%make}%{!?make:make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS"} -C src oelite

%install
[ -z %{buildroot} ] || rm -rf %{buildroot}

install -d %{buildroot}%{_sbindir}
%if 0%{?with_systemd}
%else
install -d %{buildroot}%{_initrddir}
install -d %{buildroot}%{_sysconfdir}/default
%endif
install -d %{buildroot}%{_mandir}/{man1,man8}
install -d -m 700 %{buildroot}%{_localstatedir}/omniEvents

%{?makeinstall_std:%makeinstall_std}%{!?makeinstall_std:make DESTDIR=%{buildroot} install}

install -m 755 src/oelite %{buildroot}%{_sbindir}/omniEvents
%if 0%{?with_systemd}
# install systemd unit
mkdir -p %{buildroot}%{_unitdir}
install -m 0644 %{SOURCE1} %{buildroot}%{_unitdir}
%else
install -m 775 etc/init.d/omniorb-eventservice %{buildroot}%{_initrddir}/omniEvents
install -m 644 etc/default/omniorb-eventservice %{buildroot}%{_sysconfdir}/default
%endif
install -m 644 doc/man/*.1 %{buildroot}%{_mandir}/man1
install -m 644 doc/man/omniEvents.8 %{buildroot}%{_mandir}/man8

# deprecated, but still installed by make install in 2.6.1
rm -f %{buildroot}%{_includedir}/*.{h,hh}

%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}

%pre -n %{lib_name}

%post -n %{lib_name}
/sbin/ldconfig

%if 0%{?with_systemd}
%post bootscripts
%systemd_post omniEvents.service

%preun bootscripts
%systemd_preun omniEvents.service

%postun bootscripts
%systemd_postun omniEvents.service

%else
%pre bootscripts
# A previous version is already installed?
if [ $1 -ge 2 ]; then
  /sbin/service omniEvents stop >/dev/null 2>&1
fi

%post bootscripts
/sbin/chkconfig --add omniEvents
/sbin/service omniEvents condrestart >/dev/null 2>&1

%preun bootscripts
# Are we removing the package completely?
if [ $1 -eq 0 ]; then
  /sbin/service omniEvents stop >/dev/null 2>&1
  /sbin/chkconfig --del omniEvents
fi
%endif

%postun -n %{lib_name}
/sbin/ldconfig

# main package includes libraries and copyright info
%files -n %{lib_name}
%defattr (-,root,root)
%doc LICENSE

%if 0%{?with_systemd}
%else
%config(noreplace) %attr(644,root,root) %{_sysconfdir}/default/*
%endif
%{_libdir}/*.so.*
%{_datadir}/idl/omniEvents

%files server
%defattr (-,root,root)
%dir %attr(700,omniORB,omniORB) %{_localstatedir}/omniEvents
%attr(755,root,root) %{_sbindir}/omniEvents
%attr(644,root,man) %{_mandir}/man8/*

%files bootscripts
%defattr (-,root,root)
%if 0%{?with_systemd}
%{_unitdir}/omniEvents.service
%else
%config(noreplace) %attr(775,root,root) %{_initrddir}/*
%endif

%files utils
%defattr (-,root,root)
%attr(755,root,root) %{_bindir}/*
%attr(644,root,man) %{_mandir}/man1/*

%files -n %{lib_name}-devel
%defattr(-,root,root)
%doc CHANGES* README
%{_libdir}/*.a
%{_libdir}/*.so
%{_libdir}/pkgconfig/*
%{_includedir}/omniEvents

%files doc
%defattr(-,root,root)
%doc doc/omnievents* doc/*.html doc/doxygen

%changelog
* Tue Apr 28 2015 Ryan Bauman <ryan.bauman@axiosengineering.com> 2.7.1-1
- Added systemd support

* Tue Feb 17 2015 Ryan Bauman <ryan.bauman@axiosengineering.com> 2.7.1-1
- Update version, now supports REDHAWK additions

* Tue Oct 28 2014 Ryan Bauman <ryan.bauman@axiosengineering.com> 2.6.2-9
- Don't fail if a reference to an object that is being disconnected becomes nil prematurely

* Mon Feb 10 2014 Daniel Wille <daniel.wille@axiosengineering.com> 2.6.2-8
- Use LSB logging functions in init.d script
- Use killproc in init.d to ensure omniEvents gets a SIGKILL if it won't stop

* Mon Dec 16 2013 Daniel Wille <daniel.wille@axiosengineering.com> 2.6.2-7
- Add check for omniNames reachability before starting omniEvents in service
  script.
- Remove the obsoletes

* Wed Apr 27 2005 Dirk Siebnich <dok@dok-net.net>  2.6.2-1
- better support for x86_64

* Mon Nov 15 2004 Dirk Siebnich <dok@dok-net.net>  2.6.1-1
- packaged for RPM
