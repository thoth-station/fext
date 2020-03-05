import os
import sys
from setuptools.command.test import test as TestCommand
from setuptools import Extension
from setuptools import setup


class Test(TestCommand):
    """Introduce test command to run test suite using pytest."""

    _IMPLICIT_PYTEST_ARGS = [
        "--timeout=45",
        "--capture=no",
        "--verbose",
        "-l",
        "-s",
        "-vv",
        "--hypothesis-show-statistics",
        "--random-order",
        "tests/",
    ]

    user_options = [("pytest-args=", "a", "Arguments to pass into py.test")]

    def initialize_options(self):
        super().initialize_options()
        self.pytest_args = None

    def finalize_options(self):
        super().finalize_options()
        self.test_args = []
        self.test_suite = True

    def run_tests(self):
        import pytest

        passed_args = list(self._IMPLICIT_PYTEST_ARGS)

        if self.pytest_args:
            self.pytest_args = [arg for arg in self.pytest_args.split() if arg]
            passed_args.extend(self.pytest_args)

        sys.exit(pytest.main(passed_args))


def get_version():
    with open(os.path.join("fext", "__init__.py")) as f:
        content = f.readlines()

    for line in content:
        if line.startswith("__version__ ="):
            # dirty, remove trailing and leading chars
            return line.split(" = ")[1][1:-2]
    raise ValueError("No version identifier found")


def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


VERSION = get_version()
setup(
    name="fext",
    version=VERSION,
    description="Extensions to standard Python's heapq for performance applications",
    long_description=read("README.rst"),
    author="Fridolin Pokorny",
    author_email="fridolin@redhat.com",
    url="https://github.com/thoth-station/fext",
    download_url="https://pypi.org/project/fext",
    license="GPLv3+",
    ext_modules=[
        Extension("fext.eheapq", sources=["fext/eheapq.cpp"]),
        Extension("fext.edict", sources=["fext/edict.cpp"]),
    ],
    cmdclass={"test": Test},
)
