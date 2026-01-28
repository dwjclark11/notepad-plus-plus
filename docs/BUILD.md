# Building Notepad++ on Linux

This guide covers building Notepad++ from source on Linux systems.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Distribution-Specific Instructions](#distribution-specific-instructions)
- [Build Instructions](#build-instructions)
- [Build Options](#build-options)
- [Running Tests](#running-tests)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Components

| Component | Minimum Version | Purpose |
|-----------|-----------------|---------|
| CMake | 3.16+ | Build system generator |
| Qt6 | 6.2+ | GUI framework (Core, Widgets, Gui, Network) |
| GCC/Clang | GCC 11+ / Clang 14+ | C++20 compiler |
| Python | 3.8+ | Build scripts and tooling |

### Optional Components

| Component | Purpose |
|-----------|---------|
| QScintilla2 | Alternative Scintilla implementation |
| KDE Frameworks 6 | Enhanced KDE Plasma integration |
| ccache | Faster rebuilds |
| ninja | Faster build tool (alternative to make) |

## Distribution-Specific Instructions

### Ubuntu / Debian

```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install -y build-essential cmake git

# Install Qt6 development packages
sudo apt install -y qt6-base-dev qt6-base-dev-tools \
    libqt6widgets6 libqt6gui6 libqt6network6

# Install optional dependencies
sudo apt install -y libqscintilla2-qt6-dev ccache ninja-build

# Install additional tools
sudo apt install -y pkg-config
```

### Fedora / RHEL / CentOS Stream

```bash
# Install build tools
sudo dnf install -y gcc-c++ cmake git

# Install Qt6 development packages
sudo dnf install -y qt6-qtbase-devel qt6-qtbase-gui \
    qt6-qtnetwork-devel

# Install optional dependencies
sudo dnf install -y qscintilla-qt6-devel ccache ninja-build

# Install additional tools
sudo dnf install -y pkgconfig
```

### Arch Linux / Manjaro

```bash
# Install build tools
sudo pacman -S base-devel cmake git

# Install Qt6 packages
sudo pacman -S qt6-base qt6-tools

# Install optional dependencies
sudo pacman -S qscintilla-qt6 ccache ninja
```

### openSUSE

```bash
# Install build tools
sudo zypper install -y gcc-c++ cmake git

# Install Qt6 packages
sudo zypper install -y qt6-base-devel qt6-tools

# Install optional dependencies
sudo zypper install -y qscintilla_qt6-devel ccache ninja
```

## Build Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/notepad-plus-plus/notepad-plus-plus.git
cd notepad-plus-plus
```

### 2. Configure the Build

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ../PowerEditor/src \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Or with ninja (faster)
cmake ../PowerEditor/src \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -G Ninja
```

### 3. Build

```bash
# Build with make
make -j$(nproc)

# Or with ninja
ninja

# With ccache for faster rebuilds
CCACHE_SLOPPINESS=pch_defines,time_macros ccache cmake --build . --parallel
```

### 4. Install (Optional)

```bash
# Install to system
sudo make install

# Or with ninja
sudo ninja install
```

## Build Options

### CMake Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type: Debug, Release, RelWithDebInfo, MinSizeRel |
| `CMAKE_INSTALL_PREFIX` | /usr/local | Installation prefix |
| `CMAKE_CXX_COMPILER` | system default | C++ compiler to use |
| `CMAKE_C_COMPILER` | system default | C compiler to use |

### Debug Build

```bash
cmake ../PowerEditor/src \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0" \
    -DCMAKE_INSTALL_PREFIX=/usr/local

make -j$(nproc)
```

### Release Build with Debug Info

```bash
cmake ../PowerEditor/src \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/usr/local

make -j$(nproc)
```

### Static Analysis Build

```bash
# With Clang Static Analyzer
cmake ../PowerEditor/src \
    -DCMAKE_C_COMPILER=ccc-analyzer \
    -DCMAKE_CXX_COMPILER=c++-analyzer \
    -DCMAKE_BUILD_TYPE=Debug

scan-build make -j$(nproc)
```

### Sanitizer Build (for debugging)

```bash
cmake ../PowerEditor/src \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
    -DCMAKE_LINKER_FLAGS="-fsanitize=address,undefined"

make -j$(nproc)
```

## Running Tests

### Unit Tests

```bash
# Build tests
cmake ../PowerEditor/src -DBUILD_TESTING=ON
make -j$(nproc)

# Run tests
ctest --output-on-failure
```

### Manual Testing

```bash
# Run from build directory
./notepad-plus-plus

# Test with sample files
./notepad-plus-plus /path/to/test/file.txt

# Test with multiple files
./notepad-plus-plus file1.txt file2.cpp file3.h
```

### Integration Tests

```bash
# Run the test suite
cd ../PowerEditor/Test
python3 -m pytest test_suite.py -v
```

## Troubleshooting

### Common Issues

#### Qt6 Not Found

**Error:**
```
CMake Error at CMakeLists.txt:15 (find_package):
  Could not find a package configuration file provided by "Qt6"
```

**Solution:**
```bash
# Set Qt6 directory manually
cmake ../PowerEditor/src \
    -DQt6_DIR=/usr/lib/cmake/Qt6 \
    -DCMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6
```

#### Missing Scintilla

**Error:**
```
Scintilla library not found
```

**Solution:**
The build system will automatically build Scintilla from source. If you prefer to use QScintilla:

```bash
cmake ../PowerEditor/src \
    -DQScintilla2_DIR=/usr/lib/cmake/QScintilla2
```

#### Compilation Errors with C++20

**Error:**
```
error: 'concept' does not name a type
```

**Solution:**
Ensure you have GCC 11+ or Clang 14+:

```bash
# Check GCC version
gcc --version

# Check Clang version
clang --version

# Force specific compiler
cmake ../PowerEditor/src \
    -DCMAKE_C_COMPILER=gcc-11 \
    -DCMAKE_CXX_COMPILER=g++-11
```

#### Link Errors

**Error:**
```
undefined reference to `pthread_create'
```

**Solution:**
This should be handled automatically. If not:

```bash
cmake ../PowerEditor/src \
    -DCMAKE_EXE_LINKER_FLAGS="-lpthread -ldl"
```

### Platform-Specific Issues

#### Wayland

If you experience rendering issues on Wayland, try running with XWayland:

```bash
QT_QPA_PLATFORM=xcb ./notepad-plus-plus
```

#### HiDPI Displays

For proper scaling on HiDPI displays:

```bash
QT_SCALE_FACTOR=2 ./notepad-plus-plus
# Or let Qt auto-detect
QT_AUTO_SCREEN_SCALE_FACTOR=1 ./notepad-plus-plus
```

### Getting Help

If you encounter issues not covered here:

1. Check the [FAQ](FAQ.md)
2. Search existing [GitHub Issues](https://github.com/notepad-plus-plus/notepad-plus-plus/issues)
3. Join the [Notepad++ Community](https://community.notepad-plus-plus.org/)
4. Create a new issue with:
   - Your distribution and version
   - Qt version (`qmake6 --version`)
   - Compiler version (`g++ --version`)
   - Full error output
   - Steps to reproduce

## Advanced Build Options

### Cross-Compilation

```bash
# For ARM64
cmake ../PowerEditor/src \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/arm64-linux-gnu.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

### Creating Distribution Packages

```bash
# Create DEB package
cpack -G DEB

# Create RPM package
cpack -G RPM

# Create TGZ archive
cpack -G TGZ
```

### Development Build with Full Symbols

```bash
cmake ../PowerEditor/src \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g3 -ggdb -O0" \
    -DCMAKE_C_FLAGS="-g3 -ggdb -O0"

make -j$(nproc)
```
