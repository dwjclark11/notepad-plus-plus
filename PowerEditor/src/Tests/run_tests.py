#!/usr/bin/env python3
"""
Notepad++ Linux Test Runner

This script runs the Qt Test-based test suite for the Notepad++ Linux port.
"""

import argparse
import subprocess
import sys
import os
from pathlib import Path
from typing import List, Tuple

# ANSI color codes
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

class TestRunner:
    def __init__(self, test_dir: Path):
        self.test_dir = test_dir
        self.total_tests = 0
        self.passed_tests = 0
        self.failed_tests = 0
        self.test_results: List[Tuple[str, bool, str]] = []

    def get_test_executable(self, name: str) -> Path:
        """Get the path to a test executable."""
        # Check multiple possible locations
        locations = [
            self.test_dir / "bin" / name,
            self.test_dir / name,
            Path.cwd() / "bin" / name,
            Path.cwd() / name,
        ]
        for loc in locations:
            if loc.exists():
                return loc
        return self.test_dir / "bin" / name

    def run_test(self, name: str, verbose: bool = False) -> bool:
        """Run a single test executable."""
        exe = self.get_test_executable(name)

        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print(f"Running: {name}")
        print(f"{Colors.BLUE}========================================{Colors.NC}")

        if not exe.exists():
            print(f"{Colors.YELLOW}Warning: {exe} not found{Colors.NC}")
            self.test_results.append((name, False, "Executable not found"))
            self.failed_tests += 1
            return False

        try:
            # Run test with XML output
            xml_file = f"{name}_results.xml"
            cmd = [str(exe), "-xunitxml", f"-o{xml_file}"]

            if verbose:
                result = subprocess.run(cmd, capture_output=False, text=True)
            else:
                result = subprocess.run(cmd, capture_output=True, text=True)

            if result.returncode == 0:
                print(f"{Colors.GREEN}✓ {name} passed{Colors.NC}")
                self.test_results.append((name, True, ""))
                self.passed_tests += 1
                return True
            else:
                print(f"{Colors.RED}✗ {name} failed{Colors.NC}")
                if not verbose and result.stdout:
                    print(result.stdout)
                if result.stderr:
                    print(f"{Colors.RED}stderr: {result.stderr}{Colors.NC}")
                self.test_results.append((name, False, result.stderr or "Test failed"))
                self.failed_tests += 1
                return False

        except Exception as e:
            print(f"{Colors.RED}✗ {name} error: {e}{Colors.NC}")
            self.test_results.append((name, False, str(e)))
            self.failed_tests += 1
            return False

    def run_all_tests(self, platform: bool, qtcontrols: bool, integration: bool,
                      verbose: bool = False) -> int:
        """Run all selected test categories."""
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print("Notepad++ Linux Test Suite")
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print()

        tests_to_run = []
        if platform:
            tests_to_run.append("PlatformTests")
        if qtcontrols:
            tests_to_run.append("QtControlsTests")
        if integration:
            tests_to_run.append("IntegrationTests")

        self.total_tests = len(tests_to_run)

        for test_name in tests_to_run:
            self.run_test(test_name, verbose)
            print()

        return self.print_summary()

    def print_summary(self) -> int:
        """Print test summary and return exit code."""
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print("Test Summary")
        print(f"{Colors.BLUE}========================================{Colors.NC}")
        print(f"Total test suites: {self.total_tests}")
        print(f"{Colors.GREEN}Passed: {self.passed_tests}{Colors.NC}")
        print(f"{Colors.RED}Failed: {self.failed_tests}{Colors.NC}")
        print()

        if self.failed_tests > 0:
            print(f"{Colors.RED}Failed tests:{Colors.NC}")
            for name, passed, error in self.test_results:
                if not passed:
                    print(f"  - {name}: {error}")
            print()

        if self.failed_tests == 0:
            print(f"{Colors.GREEN}All tests passed!{Colors.NC}")
            return 0
        else:
            print(f"{Colors.RED}Some tests failed!{Colors.NC}")
            return 1


def main():
    parser = argparse.ArgumentParser(
        description="Notepad++ Linux Test Runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                    Run all tests
  %(prog)s --platform-only    Run only Platform tests
  %(prog)s --verbose          Run with verbose output
  %(prog)s --list             List available tests
        """
    )

    parser.add_argument(
        "--platform-only",
        action="store_true",
        help="Run only Platform tests"
    )
    parser.add_argument(
        "--qtcontrols-only",
        action="store_true",
        help="Run only QtControls tests"
    )
    parser.add_argument(
        "--integration-only",
        action="store_true",
        help="Run only Integration tests"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose output"
    )
    parser.add_argument(
        "--list", "-l",
        action="store_true",
        help="List available tests"
    )
    parser.add_argument(
        "--test-dir",
        type=Path,
        default=None,
        help="Directory containing test executables"
    )

    args = parser.parse_args()

    # Determine test directory
    if args.test_dir:
        test_dir = args.test_dir
    else:
        # Try to find test directory
        script_dir = Path(__file__).parent
        test_dir = script_dir

    if args.list:
        print("Available tests:")
        print("  - PlatformTests")
        print("  - QtControlsTests")
        print("  - IntegrationTests")
        return 0

    # Determine which tests to run
    run_platform = True
    run_qtcontrols = True
    run_integration = True

    if args.platform_only:
        run_qtcontrols = False
        run_integration = False
    elif args.qtcontrols_only:
        run_platform = False
        run_integration = False
    elif args.integration_only:
        run_platform = False
        run_qtcontrols = False

    # Create runner and execute tests
    runner = TestRunner(test_dir)
    return runner.run_all_tests(
        run_platform,
        run_qtcontrols,
        run_integration,
        args.verbose
    )


if __name__ == "__main__":
    sys.exit(main())
