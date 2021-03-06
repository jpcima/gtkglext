# -*- rpm-spec -*-

%define base_version		1.2.0
%define api_version		1.0
%define rel			1

Summary: OpenGL Extension to GTK
Name: gtkglext
Version: %{base_version}
Release: %{rel}
License: LGPL
Group: System Environment/Libraries
URL: http://gtkglext.sourceforge.net/
Source0: ftp://dl.sourceforge.net/pub/sourceforge/gtkglext/gtkglext-%{version}.tar.gz
BuildRoot: %{_tmppath}/gtkglext-%{version}-root

Requires: gtk2
Requires: XFree86-libs

BuildRequires: gtk2-devel
BuildRequires: XFree86-devel
BuildRequires: pkgconfig

%description
GtkGLExt is an OpenGL extension to GTK. It provides the GDK objects
which support OpenGL rendering in GTK, and GtkWidget API add-ons to
make GTK+ widgets OpenGL-capable.

%package devel
Summary: Development tools for GTK-based OpenGL applications
Group: Development/Libraries

Requires: %{name} = %{version}
Requires: gtk2-devel
Requires: XFree86-devel

%description devel
The gtkglext-devel package contains the header files, static libraries,
and developer docs for GtkGLExt.

%prep
%setup -q -n gtkglext-%{version}

%build
%configure --disable-gtk-doc
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

pushd "$RPM_BUILD_ROOT%{_libdir}"

cp -p libgdkglext-x11-%{api_version}.la libgdkglext-x11-%{api_version}.la.bak
cat libgdkglext-x11-%{api_version}.la.bak | \
  sed -e "s| -L$RPM_BUILD_ROOT%{_libdir}||g" > libgdkglext-x11-%{api_version}.la
rm -f libgdkglext-x11-%{api_version}.la.bak

cp -p libgtkglext-x11-%{api_version}.la libgtkglext-x11-%{api_version}.la.bak
cat libgtkglext-x11-%{api_version}.la.bak | \
  sed -e "s| -L$RPM_BUILD_ROOT%{_libdir}||g" > libgtkglext-x11-%{api_version}.la
rm -f libgtkglext-x11-%{api_version}.la.bak

popd

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)

%doc AUTHORS COPYING COPYING.LIB ChangeLog NEWS README TODO
%{_libdir}/libgdkglext-x11-%{api_version}.so.*
%{_libdir}/libgtkglext-x11-%{api_version}.so.*

%files devel
%defattr(-,root,root,-)

%{_includedir}/*
%{_libdir}/gtkglext-%{api_version}
%{_libdir}/lib*.so
%{_libdir}/lib*.a
%{_libdir}/lib*.la
%{_libdir}/pkgconfig/*
%{_datadir}/aclocal/*
%{_datadir}/gtk-doc/html/*

%changelog
* Sun Aug 31 2003 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Updated source URL.

* Sun May 11 2003 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Removed LDFLAGS setting.
- Removed atk, pango, glib2 from Requires.
- Remove lib*.la.bak files.

* Mon Feb 24 2003 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Added %{_datadir}/aclocal/* to the file list.
- Re-enabled static libraries by default.

* Tue Dec  3 2002 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Added %{_libdir}/gtkglext-%{api_version} to the file list.

* Fri Nov 15 2002 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Removed --disable-mesa-ext configure option.
- Disabled static libraries by default.

* Sat Aug  3 2002 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Added --disable-mesa-ext configure option.

* Sun Jun 23 2002 Naofumi Yasufuku <naofumi@users.sourceforge.net>
- Initial build.


