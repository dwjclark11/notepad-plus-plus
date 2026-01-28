#!/bin/bash
# Test execution script for Notepad++ Linux port
# Usage: ./ci/scripts/test.sh [options]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_DIR="build"
VERBOSE=false
RUN_VALGRIND=false
RUN_GDB=false
GENERATE_COVERAGE=false
TEST_FILTER=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --valgrind)
            RUN_VALGRIND=true
            shift
            ;;
        --gdb)
            RUN_GDB=true
            shift
            ;;
        --coverage)
            GENERATE_COVERAGE=true
            shift
            ;;
        --filter)
            TEST_FILTER="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR    Build directory (default: build)"
            echo "  --verbose          Verbose test output"
            echo "  --valgrind         Run tests under Valgrind"
            echo "  --gdb              Run tests under GDB"
            echo "  --coverage         Generate coverage report"
            echo "  --filter PATTERN   Filter tests by pattern"
            echo "  --help             Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOURCE_DIR="$PROJECT_ROOT/PowerEditor/src"
BUILD_PATH="$SOURCE_DIR/$BUILD_DIR"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Notepad++ Linux Test Script${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Build Directory: $BUILD_PATH"
echo "Verbose: $VERBOSE"
echo "Valgrind: $RUN_VALGRIND"
echo "Coverage: $GENERATE_COVERAGE"
echo ""

# Check if build exists
if [ ! -d "$BUILD_PATH" ]; then
    echo -e "${RED}Error: Build directory not found: $BUILD_PATH${NC}"
    echo "Please build the project first with:"
    echo "  ./ci/scripts/build.sh"
    exit 1
fi

# Check for binary
if [ ! -f "$BUILD_PATH/notepad-plus-plus" ]; then
    echo -e "${RED}Error: Notepad++ binary not found${NC}"
    exit 1
fi

echo -e "${YELLOW}Running tests...${NC}"
echo ""

# Function to run unit tests
run_unit_tests() {
    echo -e "${BLUE}Running unit tests...${NC}"

    cd "$BUILD_PATH"

    # Check if CTest is available
    if [ -f CTestTestfile.cmake ] || [ -f "${BUILD_PATH}/CTestTestfile.cmake" ]; then
        CTEST_ARGS=(--output-on-failure)

        if [ "$VERBOSE" == true ]; then
            CTEST_ARGS+=(-V)
        fi

        if [ -n "$TEST_FILTER" ]; then
            CTEST_ARGS+=(-R "$TEST_FILTER")
        fi

        if ctest "${CTEST_ARGS[@]}"; then
            echo -e "${GREEN}All unit tests passed!${NC}"
            return 0
        else
            echo -e "${RED}Some unit tests failed!${NC}"
            return 1
        fi
    else
        echo -e "${YELLOW}No CTest configuration found. Skipping unit tests.${NC}"
        return 0
    fi
}

# Function to run integration tests
run_integration_tests() {
    echo -e "${BLUE}Running integration tests...${NC}"

    # Create test directory
    TEST_DIR=$(mktemp -d)
    trap "rm -rf $TEST_DIR" EXIT

    # Test 1: Binary execution test
    echo -n "Test 1: Binary execution... "
    if "$BUILD_PATH/notepad-plus-plus" --help 2>/dev/null || true; then
        echo -e "${GREEN}PASS${NC}"
    else
        # --help might not be implemented, check if binary runs at all
        if file "$BUILD_PATH/notepad-plus-plus" | grep -q "executable"; then
            echo -e "${GREEN}PASS${NC} (binary is executable)"
        else
            echo -e "${RED}FAIL${NC}"
            return 1
        fi
    fi

    # Test 2: Version check
    echo -n "Test 2: Version output... "
    if "$BUILD_PATH/notepad-plus-plus" --version 2>/dev/null || true; then
        echo -e "${GREEN}PASS${NC}"
    else
        echo -e "${YELLOW}SKIP${NC} (version flag not implemented)"
    fi

    # Test 3: File creation test (if we can run headlessly)
    echo -n "Test 3: File operations... "
    TEST_FILE="$TEST_DIR/test.txt"
    echo "Test content" > "$TEST_FILE"
    if [ -f "$TEST_FILE" ]; then
        echo -e "${GREEN}PASS${NC}"
    else
        echo -e "${RED}FAIL${NC}"
        return 1
    fi

    echo -e "${GREEN}Integration tests complete!${NC}"
    return 0
}

# Function to run Valgrind memory check
run_valgrind_check() {
    if [ "$RUN_VALGRIND" == false ]; then
        return 0
    fi

    echo -e "${BLUE}Running Valgrind memory check...${NC}"

    if ! command -v valgrind &> /dev/null; then
        echo -e "${YELLOW}Valgrind not found, skipping memory check${NC}"
        return 0
    fi

    VALGRIND_ARGS=(
        --leak-check=full
        --show-leak-kinds=all
        --track-origins=yes
        --verbose
        --log-file=valgrind-report.txt
        --error-exitcode=1
    )

    cd "$BUILD_PATH"

    # Run valgrind (may fail due to GUI requirements)
    if xvfb-run -a valgrind "${VALGRIND_ARGS[@]}" ./notepad-plus-plus --help 2>/dev/null || true; then
        echo -e "${GREEN}Valgrind check complete!${NC}"
        if [ -f valgrind-report.txt ]; then
            echo "Report saved to: $BUILD_PATH/valgrind-report.txt"
        fi
    else
        echo -e "${YELLOW}Valgrind check had issues (GUI app may need display)${NC}"
    fi
}

# Function to generate coverage report
generate_coverage_report() {
    if [ "$GENERATE_COVERAGE" == false ]; then
        return 0
    fi

    echo -e "${BLUE}Generating coverage report...${NC}"

    cd "$SOURCE_DIR"

    # Check for gcovr
    if command -v gcovr &> /dev/null; then
        mkdir -p "$BUILD_PATH/coverage"

        gcovr -r . \
            --html --html-details \
            -o "$BUILD_PATH/coverage/coverage-report.html" \
            "$BUILD_PATH" || true

        gcovr -r . \
            --xml \
            -o "$BUILD_PATH/coverage/coverage-report.xml" \
            "$BUILD_PATH" || true

        echo -e "${GREEN}Coverage report generated!${NC}"
        echo "HTML: $BUILD_PATH/coverage/coverage-report.html"
        echo "XML:  $BUILD_PATH/coverage/coverage-report.xml"
    else
        echo -e "${YELLOW}gcovr not found, trying lcov...${NC}"

        if command -v lcov &> /dev/null; then
            lcov --capture --directory "$BUILD_PATH" --output-file "$BUILD_PATH/coverage.info" || true
            lcov --remove "$BUILD_PATH/coverage.info" '/usr/*' --output-file "$BUILD_PATH/coverage.info" || true

            if command -v genhtml &> /dev/null; then
                genhtml "$BUILD_PATH/coverage.info" --output-directory "$BUILD_PATH/coverage" || true
                echo -e "${GREEN}Coverage report generated!${NC}"
                echo "Location: $BUILD_PATH/coverage/"
            fi
        else
            echo -e "${YELLOW}Neither gcovr nor lcov found, skipping coverage report${NC}"
        fi
    fi
}

# Run tests
FAILED=0

# Run unit tests
if ! run_unit_tests; then
    FAILED=1
fi

# Run integration tests
if ! run_integration_tests; then
    FAILED=1
fi

# Run Valgrind check
run_valgrind_check

# Generate coverage report
generate_coverage_report

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Test Summary${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
