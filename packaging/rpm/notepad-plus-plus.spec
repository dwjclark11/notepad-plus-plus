Name:           notepad-plus-plus
Version:        8.6.0
Release:        1%{?dist}
Summary:        Free source code editor and Notepad replacement

License:        GPLv2+
URL:            https://notepad-plus-plus.org/
Source0:        https://github.com/notepad-plus-plus/notepad-plus-plus/archive/v%{version}/%{name}-%{version}.tar.gz

# Desktop file
Source1:        notepad-plus-plus.desktop
# MIME type definition
Source2:        notepad-plus-plus-mime.xml
# AppData/metainfo
Source3:        org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml
# Icon
Source4:        notepad-plus-plus.svg

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++ >= 11
BuildRequires:  qt6-qtbase-devel >= 6.2
BuildRequires:  qt6-qttools-devel
BuildRequires:  pkgconfig
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  hicolor-icon-theme
BuildRequires:  mesa-libGL-devel
BuildRequires:  vulkan-devel

Requires:       qt6-qtbase >= 6.2
Requires:       qt6-qtbase-gui
Requires:       qt6-qtnetwork
Requires:       hicolor-icon-theme

Recommends:     %{name}-lang

%description
Notepad++ is a free source code editor and Notepad replacement that supports
several languages. Running in the MS Windows environment, its use is governed
by GPL License.

Based on the powerful editing component Scintilla, Notepad++ is written in
C++ and uses pure Win32 API and STL which ensures a higher execution speed
and smaller program size. By optimizing as many routines as possible without
losing user friendliness, Notepad++ is trying to reduce the world carbon
dioxide emissions. When using less CPU power, the PC can throttle down and
reduce power consumption, resulting in a greener environment.

This package provides the Linux port of Notepad++ using Qt6 for the user
interface.

%package lang
Summary:        Localization files for Notepad++
BuildArch:      noarch
Requires:       %{name} = %{version}-%{release}

%description lang
This package contains the localization files for Notepad++.

%package plugins
Summary:        Plugin support for Notepad++
Requires:       %{name} = %{version}-%{release}

%description plugins
This package provides additional plugins for Notepad++.

%prep
%autosetup -p1

# Copy additional source files
cp %{SOURCE1} PowerEditor/src/
cp %{SOURCE2} PowerEditor/src/
cp %{SOURCE3} PowerEditor/src/
cp %{SOURCE4} PowerEditor/src/ 2>/dev/null || true

%build
# Create build directory
mkdir -p build
cd build

# Configure with CMake
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    ../PowerEditor/src

# Build
%cmake_build

%install
cd build
%cmake_install

# Install desktop file
desktop-file-install \
    --dir %{buildroot}%{_datadir}/applications \
    --set-icon=%{name} \
    %{SOURCE1}

# Install icons
install -Dm 644 %{SOURCE4} \
    %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg 2>/dev/null || \
install -d %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/

# Install metainfo
install -Dm 644 %{SOURCE3} \
    %{buildroot}%{_datadir}/metainfo/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml

# Install MIME type
install -Dm 644 %{SOURCE2} \
    %{buildroot}%{_datadir}/mime/packages/%{name}.xml

# Install localization files
install -d %{buildroot}%{_datadir}/%{name}/localization
if [ -d PowerEditor/installer/nativeLang ]; then
    cp -r PowerEditor/installer/nativeLang/* \
        %{buildroot}%{_datadir}/%{name}/localization/ 2>/dev/null || true
fi

# Install themes
install -d %{buildroot}%{_datadir}/%{name}/themes
if [ -d PowerEditor/installer/themes ]; then
    cp -r PowerEditor/installer/themes/* \
        %{buildroot}%{_datadir}/%{name}/themes/ 2>/dev/null || true
fi

# Create symlinks
ln -sf %{_bindir}/notepad-plus-plus %{buildroot}%{_bindir}/notepad++

%check
# Verify desktop file
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop || :

# Verify AppData
appstream-util validate-relax --nonet \
    %{buildroot}%{_datadir}/metainfo/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml || :

%post
# Update MIME database
if [ -x /usr/bin/update-mime-database ]; then
    update-mime-database /usr/share/mime &> /dev/null || :
fi

# Update desktop database
if [ -x /usr/bin/update-desktop-database ]; then
    update-desktop-database /usr/share/applications &> /dev/null || :
fi

# Update icon cache
if [ -x /usr/bin/gtk-update-icon-cache ]; then
    gtk-update-icon-cache -f /usr/share/icons/hicolor &> /dev/null || :
fi

%postun
# Update MIME database
if [ -x /usr/bin/update-mime-database ]; then
    update-mime-database /usr/share/mime &> /dev/null || :
fi

# Update desktop database
if [ -x /usr/bin/update-desktop-database ]; then
    update-desktop-database /usr/share/applications &> /dev/null || :
fi

# Update icon cache
if [ -x /usr/bin/gtk-update-icon-cache ]; then
    gtk-update-icon-cache -f /usr/share/icons/hicolor &> /dev/null || :
fi

%files
%license LICENSE
%doc README.md BUILD.md CONTRIBUTING.md
%{_bindir}/notepad-plus-plus
%{_bindir}/notepad++
%{_datadir}/applications/%{name}.desktop
%{_datadir}/mime/packages/%{name}.xml
%{_datadir}/metainfo/org.notepad-plus-plus.NotepadPlusPlus.metainfo.xml
%{_datadir}/%{name}/themes/
%dir %{_datadir}/%{name}/

%files lang
%{_datadir}/%{name}/localization/

%files plugins
%dir %{_libdir}/%{name}/
%dir %{_libdir}/%{name}/plugins/

%changelog
* Mon Jan 01 2024 Notepad++ Team <don.h@free.fr> - 8.6.0-1
- Initial release of Linux port
- Qt6-based user interface
- Native Linux file dialogs and platform integration
- Scintilla and Lexilla integration
- Plugin architecture support

* Thu Nov 02 2023 Notepad++ Team <don.h@free.fr> - 8.5.8-1
- Placeholder for future release
