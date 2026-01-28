# Notepad++ Linux Port CI/CD Pipeline

This directory contains the CI/CD configuration for the Notepad++ Linux port.

## Overview

The CI/CD pipeline provides:

- **Multi-distribution builds**: Ubuntu, Fedora, Arch Linux
- **Multiple compilers**: GCC and Clang
- **Code quality checks**: clang-format, clang-tidy, cppcheck
- **Test execution**: Unit tests and integration tests
- **Package generation**: Tarball, Debian, RPM, AppImage, Flatpak
- **Docker support**: Local CI simulation

## GitHub Actions Workflows

### 1. Linux Build (`linux-build.yml`)

Builds Notepad++ on multiple Linux distributions:

- **Ubuntu**: 22.04, 24.04
- **Fedora**: Latest
- **Arch Linux**: Latest
- **Compilers**: GCC, Clang
- **Build types**: Release, Debug

Jobs:
- `build-ubuntu`: Build on Ubuntu with matrix strategy
- `build-fedora`: Build on Fedora
- `build-arch`: Build on Arch Linux
- `test`: Run test suite
- `coverage`: Generate code coverage reports

### 2. Windows Build (`windows-build.yml`)

Verifies Windows compatibility:

- **MSVC**: Visual Studio 2022 (x64, Win32, ARM64)
- **MinGW**: MSYS2 (x86_64, i686)
- **CMake**: Windows CMake builds

### 3. Code Quality (`code-quality.yml`)

Runs code quality checks:

- `clang-format`: Code formatting verification
- `clang-tidy`: Static analysis with clang-tidy
- `cppcheck`: Static analysis with cppcheck
- `coverage-summary`: Coverage reporting on PRs
- `quality-gate`: Overall quality assessment

### 4. Release (`release.yml`)

Creates release packages:

- `build-linux-release`: Optimized release builds
- `build-appimage`: AppImage package
- `build-flatpak`: Flatpak package
- `build-debian`: Debian package
- `build-rpm`: RPM package
- `create-release`: GitHub release with artifacts

## Docker Support

### Build Images

Build Docker images for local development:

```bash
# Ubuntu
docker build -f ci/docker/Dockerfile.ubuntu -t notepad-plus-plus-build:ubuntu .

# Fedora
docker build -f ci/docker/Dockerfile.fedora -t notepad-plus-plus-build:fedora .

# Arch Linux
docker build -f ci/docker/Dockerfile.arch -t notepad-plus-plus-build:arch .
```

### Using Docker Compose

```bash
# Start Ubuntu build environment
docker-compose run ubuntu

# Build in container
docker-compose run --rm ubuntu build Release

# Run code quality checks
docker-compose run --rm code-quality

# Build and test
docker-compose run --rm build-test
```

### Docker Commands

```bash
# Interactive shell
docker-compose run ubuntu shell

# Build with specific compiler
docker-compose run ubuntu-clang build Release

# Run tests
docker-compose run ubuntu test

# Generate coverage
docker-compose run ubuntu coverage
```

## Build Scripts

### `build.sh`

Main build script for Linux:

```bash
# Build Release (default)
./ci/scripts/build.sh

# Build Debug
./ci/scripts/build.sh Debug

# Clean build with Clang
./ci/scripts/build.sh Release --clean --clang

# Build with coverage
./ci/scripts/build.sh --coverage

# Verbose build
./ci/scripts/build.sh --verbose
```

Options:
- `--clean`: Clean build directory
- `--verbose`: Verbose output
- `--coverage`: Coverage instrumentation
- `--clang`: Use Clang
- `--gcc`: Use GCC (default)
- `-j N`: Parallel jobs

### `test.sh`

Test execution script:

```bash
# Run all tests
./ci/scripts/test.sh

# Run with Valgrind
./ci/scripts/test.sh --valgrind

# Generate coverage report
./ci/scripts/test.sh --coverage

# Verbose output
./ci/scripts/test.sh --verbose
```

Options:
- `--build-dir DIR`: Build directory
- `--verbose`: Verbose output
- `--valgrind`: Run under Valgrind
- `--gdb`: Run under GDB
- `--coverage`: Generate coverage
- `--filter PATTERN`: Filter tests

### `package.sh`

Package generation script:

```bash
# Create all packages
./ci/scripts/package.sh --type all

# Create specific package
./ci/scripts/package.sh --type deb
./ci/scripts/package.sh --type rpm
./ci/scripts/package.sh --type appimage
./ci/scripts/package.sh --type tarball

# Custom version
./ci/scripts/package.sh --type deb --version 8.6.0-1
```

Options:
- `--type TYPE`: Package type (all, tarball, deb, rpm, appimage, flatpak)
- `--version VER`: Package version
- `--arch ARCH`: Target architecture
- `--output DIR`: Output directory

## Local CI Simulation

### Full CI Pipeline Locally

```bash
# 1. Build Docker images
docker-compose build

# 2. Run code quality checks
docker-compose run --rm code-quality

# 3. Build
docker-compose run --rm build-test

# 4. Create packages
./ci/scripts/package.sh --type all
```

### Manual Build Steps

```bash
# Install dependencies (Ubuntu example)
sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev qscintilla2-qt6-dev

# Build
./ci/scripts/build.sh Release

# Test
./ci/scripts/test.sh

# Package
./ci/scripts/package.sh --type tarball
```

## CI/CD Variables

### Environment Variables

- `BUILD_TYPE`: Release or Debug
- `QT_VERSION`: Qt version (6.6)
- `CC`: C compiler
- `CXX`: C++ compiler

### Secrets

- `CODECOV_TOKEN`: Codecov upload token
- `GITHUB_TOKEN`: GitHub release token (auto-provided)

## Troubleshooting

### Build Failures

1. **Missing Qt6**: Install Qt6 development packages
2. **Missing QScintilla**: Install QScintilla2 Qt6 packages
3. **CMake errors**: Ensure CMake >= 3.16
4. **Ninja not found**: Install ninja-build or use `cmake --build`

### Test Failures

1. **Display issues**: Tests may need Xvfb for GUI components
2. **Missing test data**: Ensure test files are present
3. **Valgrind errors**: Some Qt warnings are expected

### Package Creation Issues

1. **AppImage**: Requires linuxdeployqt
2. **Flatpak**: Requires flatpak-builder
3. **RPM**: Requires rpmbuild (Fedora/RHEL environment)
4. **Debian**: Requires dpkg-deb

## Contributing

When adding new CI features:

1. Test locally with Docker first
2. Update this README
3. Ensure scripts are executable
4. Add appropriate error handling
5. Update workflow triggers if needed

## References

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Docker Documentation](https://docs.docker.com/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Qt6 Documentation](https://doc.qt.io/qt-6/)
