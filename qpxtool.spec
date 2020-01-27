%define summary  CD/DVD media check/drive control tools
%define name     qpxtool
%define version  0.7.2
%define release  1
%define vendor   Gennady "ShultZ" Kozlov
%define packager Gennady "ShultZ" Kozlov
%define email    qpxtool@mail.ru

Summary: %{summary}
Name: %{name}
Version: %{version}
Release: %{release}
Group: Applications/Media
License: GPL
URL: http://qpxtool.sourceforge.net
Vendor: %{vendor}
Packager: %{packager} %{email}
Source: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-%(%{__id_u} -n)

#BuildRequires:  libpng-devel
Requires:	%{name}-lib = %{version}-%{release}
#Requires:	libpng

%package lib
Summary:	QPxTool libraries
Group:		Development/Libraries

%package gui
Summary:	QPxTool GUI
Group:		X11/Applications
Requires:	%{name} = %{version}-%{release}
Requires:	%{name}-lib = %{version}-%{release}

%package devel
Summary:	QPxTool development files
Group:		Development/Libraries
Requires:	%{name}-lib = %{version}-%{release}

%description
QPxTool is the way to get full control over your CD/DVD drives.
It is the Open Source Solution which intends to give you access to all
available Quality Checks (Q-Checks) on written and blank media, that
are available for your drive. This will help you to find the right media
and the optimized writing speed for your hardware, which will increase
the chance for a long data lifetime.

Console CD/DVD media check and drive control tools and libraries

qscan       - quality check tool
qscand      - network quality check daemon (frontend to qscan)
readdvd     - a little tool for reading damaged CD/DVD (multi-pass)
cdvdcontrol - extended drive features control tool
pxfw        - firmware flasher for Plextor drives
f1tattoo    - DiscT@2 tool for Yamaha CRW-F1

Authors:
--------
    Gennady "ShultZ" Kozlov <qpxtool@mail.ru>

%description lib
Required libraries for QPxTool CD/DVD media check and drive control tools

Authors:
--------
    Gennady "ShultZ" Kozlov <qpxtool@mail.ru>

%description devel
Developnemt files for QPxTool CD/DVD media check and drive control tools

Authors:
--------
    Gennady "ShultZ" Kozlov <qpxtool@mail.ru>

%description gui
Qt frontend for CD/DVD media check and drive control tools

Authors:
--------
    Gennady "ShultZ" Kozlov <qpxtool@mail.ru>

%prep
%setup -q -n %{name}

%build
####
./configure --prefix=/usr --libdir=%{_libdir} --mandir=%{_mandir}
make %{?_smp_mflags}

%install

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

%post lib
/sbin/ldconfig

%postun lib
/sbin/ldconfig

%files
%defattr(-,root,root)
/usr/bin/*
/usr/sbin/*
%{_mandir}/man1/*
%{_mandir}/man8/*
%{_libdir}/qpxtool/libqscan_*
%doc AUTHORS COPYING ChangeLog README TODO
%exclude /usr/bin/qpxtool

%files lib
%defattr(-,root,root)
%{_libdir}/libqpx*

%files gui
%defattr(-,root,root)
/usr/bin/qpxtool
/usr/share/pixmaps/qpxtool.png
/usr/share/applications/qpxtool.desktop
/usr/share/qpxtool/locale/*

%files devel
%defattr(-,root,root)
/usr/include/qpxtool/*

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && %__rm -rf $RPM_BUILD_ROOT

%changelog
* Wed Nov 11 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
- main package splitted to main,lib

* Wed Oct 21 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
- splitted packages: main, devel, gui

* Tue Jul 21 2009 Gennady "ShultZ" Kozlov <qpxtool@mail.ru>
- initial spec file created
