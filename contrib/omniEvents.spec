%define _name omniEvents
%define srcversion 2_6_2

%define lib_name %{?mklibname:%mklibname %{_name} 2}%{!?mklibname:lib%{_name}2}

Summary: CORBA Event Service for omniORB
Name:    %{_name}
Version: 2.6.2
Release: 1
License: LGPL
Group:   System/Libraries
Source0: %{_name}-%{srcversion}-src.tar.gz
URL:     http://www.omnievents.org/
BuildRequires:  libomniorb-devel glibc-devel
Buildroot:      %{_tmppath}/%{name}-%{version}-root

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
Obsoletes:      omniEvents

%description -n %{lib_name}
%{name} enables CORBA applications to communicate through asynchronous
broadcast channels rather than direct method calls.
It is a small, efficient implementation of the Object Management Group's
Event Service specification designed to work with omniORB.

%package server
Summary:   CORBA Event Service daemon
Group:     Development/C++
Obsoletes: omniEvents-server
Requires: %{lib_name} = %{version}-%{release}

%description server
The CORBA Event Service daemon as a standalone executable.

%package bootscripts
Summary:   Utility programs
Group:     Development/C++
%if "%{_vendor}" == "suse"
Prereq:         /sbin/insserv
%else
Prereq:         /sbin/service /sbin/chkconfig
%endif
Obsoletes: omniEvents-bootscripts
Requires:  %{name}-server = %{version}-%{release} %{name}-utils = %{version}-%{release}

%description bootscripts
Automatic starting of the %{name} CORBA Event Service.

%package utils
Summary:   Utility programs
Group:     Development/C++
Obsoletes: omniEvents-utils
Requires:  %{lib_name} = %{version}-%{release}

%description utils
%{name} utility programs which may be useful at runtime.

%package -n %{lib_name}-devel
Summary: Header files and libraries needed for %{name} development
Group:          Development/C++
Requires:       %{lib_name} = %{version}-%{release}
Obsoletes:      omniEvents-devel
Provides:       libomnievents-devel = %{version}-%{release} %{name}-devel = %{version}-%{release}
Conflicts:      libomnievents-devel > %{version}-%{release} libomnievents-devel < %{version}-%{release}

%description -n %{lib_name}-devel
The header files and libraries needed for developing programs using %{name}.

%package doc
Summary: Documentation and examples for %{name}
Group:          Development/C++
Obsoletes:      omniEvents-doc
#Requires:       %{lib_name} = %{version}-%{release}

%description doc
Developer documentation and examples.

%prep
%setup -n %{_name}-%{srcversion}

%{?configure:%configure}%{!?configure:CPPFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix} --libdir=%{_libdir}} --enable-unloadable-stubs

%build
%{?make:%make}%{!?make:make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS"} all
%{?make:%make}%{!?make:make IMPORT_CPPFLAGS+="$RPM_OPT_FLAGS"} -C src oelite

%install
[ -z %{buildroot} ] || rm -rf %{buildroot}

install -d %{buildroot}%{_sbindir}
install -d %{buildroot}%{_initrddir}
install -d %{buildroot}%{_sysconfdir}/default
install -d %{buildroot}%{_mandir}/{man1,man8}
install -d -m 700 %{buildroot}%{_localstatedir}/omniEvents

%{?makeinstall_std:%makeinstall_std}%{!?makeinstall_std:make DESTDIR=%{buildroot} install}

install -m 755 src/oelite %{buildroot}%{_sbindir}/omniEvents
install -m 775 etc/init.d/omniorb-eventservice %{buildroot}%{_initrddir}/omniEvents
install -m 644 etc/default/omniorb-eventservice %{buildroot}%{_sysconfdir}/default
install -m 644 doc/man/*.1 %{buildroot}%{_mandir}/man1
install -m 644 doc/man/omniEvents.8 %{buildroot}%{_mandir}/man8

# deprecated, but still installed by make install in 2.6.1
rm -f %{buildroot}%{_includedir}/*.{h,hh}

%if "%{_vendor}" == "suse"
  # Most SUSE service scripts have a corresponding link into /usr/sbin
  ln -sf %{_initrddir}/omniEvents %{buildroot}%{_sbindir}/rcomniEvents
%endif

%clean
[ -z %{buildroot} ] || rm -rf %{buildroot}

%pre -n %{lib_name}

%post -n %{lib_name}
/sbin/ldconfig

%pre bootscripts
# A previous version is already installed?
if [ $1 -ge 2 ]; then
%if "%{_vendor}" == "suse"
  %{_sbindir}/rcomniEvents stop >/dev/null 2>&1
%else
  /sbin/service omniEvents stop >/dev/null 2>&1
%endif
fi

%post bootscripts
%if "%{_vendor}" == "suse"
/sbin/insserv omniEvents
%{_sbindir}/rcomniEvents restart >/dev/null 2>&1
%else
/sbin/chkconfig --add omniEvents
/sbin/service omniEvents restart >/dev/null 2>&1
%endif

%preun bootscripts
# Are we removing the package completely?
if [ $1 -eq 0 ]; then
%if "%{_vendor}" == "suse"
  %{_sbindir}/rcomniEvents stop >/dev/null 2>&1
  /sbin/insserv -r omniEvents
%else
  /sbin/service omniEvents stop >/dev/null 2>&1
  /sbin/chkconfig --del omniEvents
%endif
fi

%postun -n %{lib_name}
/sbin/ldconfig

# main package includes libraries and copyright info
%files -n %{lib_name}
%defattr (-,root,root)
%doc LICENSE
%config(noreplace) %attr(644,root,root) %{_sysconfdir}/default/*
%{_libdir}/*.so.*
%{_datadir}/idl/omniEvents

%files server
%defattr (-,root,root)
%dir %attr(700,omni,omni) %{_localstatedir}/omniEvents
%attr(755,root,root) %{_sbindir}/omniEvents
%attr(644,root,man) %{_mandir}/man8/*

%files bootscripts
%defattr (-,root,root)
%config(noreplace) %attr(775,root,root) %{_initrddir}/*
%if "%{_vendor}" == "suse"
%{_sbindir}/rcomniEvents
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
* Wed Apr 27 2005 Dirk Siebnich <dok@dok-net.net>  2.6.2-1
- better support for x86_64

* Mon Nov 15 2004 Dirk Siebnich <dok@dok-net.net>  2.6.1-1
- packaged for RPM
