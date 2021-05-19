fext
----

Fast CPython extensions to Python standard library with focus on performance.

This library provides CPython native extensions to mimic some of the well known
built-in types. The implementation relies on enforced protocol - all the
objects and abstract data types are implemented in C/C++ to provide highly
effective manipulation.

See `this video for more info <https://youtu.be/B7GsCVFpaXo?t=2235>`__.

Extended heapq - fext.ExtHeapQueue
==================================

The extended heap queue acts as a min-heap queue from the standard Python
library.  It uses a hash table for storing information about indexes (where
values sit in the min-heap queue) to optimize removals from the heap
to O(log(N)) in comparision to the original O(N+N*log(N)).

.. figure:: https://raw.githubusercontent.com/thoth-station/fext/master/fig/fext_extheapq.png
   :scale: 40%
   :align: center

Using fext in a C++ project
===========================

The design of this library allows you to use sources in your C++ project as
well. The ``eheapq.hpp`` file defines the extended heap queue and ``edict.hpp`` the
extended dictionary. Python files then act as a bindings to their respective
Python interfaces. Mind the API design for the templated classes - it was meant to
be used with pointers to objects (so avoid possible copy constructors).

Building the extensions
=======================

To build extensions, install the following packages (Fedora):

.. code-block:: console

  dnf install -y python3-devel g++ gcc

Now you can build extensions:

.. code-block:: console

  python3 setup.py build

If you would like to produce binaries with debug information:

.. code-block:: console

  CFLAGS='-Wall -O0 -ggdb' python3 setup.py build

Check sections below for more info on testing the C/C++ parts of extensions.

When building extension for a more recent Python releases (e.g. Python 3.8) build the extension
inside the container image so that it provides required libraries with the required symbols:

```console
cd fext/
podman run --rm --workdir /io --entrypoint bash -it --volume `pwd`:/io:Z quay.io/pypa/manylinux2014_x86_64
yum install -y rh-python38-python-setuptools-wheel rh-python38-python rh-python38-python-devel rh-python38-python-wheel
scl enable rh-python38 bash
python3 setup.py bdist_wheel
auditwheel repair dist/*
twine upload dist/*
```

Reference count and memory leak checks
======================================

You can find ``Makefile`` in the Git repo. This repo defines targets to
perform leak checks and reference count checks. Note they use different Python
interpreters (with/without debug information) so make sure you do not mix
virtual environments when running the tests.

.. code-block:: console

  make check

Developing the extension
========================

First, prepare your environment:

.. code-block:: console

  dnf install -y make
  make deps

To develop or adjust sources, simply change sources and verify your
change is accepted by the test suite:

.. code-block::

  make check

The ``check`` target will run the actual test suite (see also ``make test``).
Besides it, the test suite will be executed two more times to check test suite
and its behaviour with respect to Python object reference counting
(``python3-debug`` dependency will be automatically installed with the provided
``make deps``). This part of the test suite can be executed using ``make
check-refcount``. The last part of the test suite runs valgrind against the
test suite - you can explicitly trigger this part by calling ``make
check-leaks``.

Mind ``make check-refcount`` and ``make check-leaks`` will take some time given the
checks and processing that is done on the background. To verify your changes
more iterativelly, ``make test`` should do the trick (don't forget to do ``make
check`` after that though).

To clean up your environment, perform:

.. code-block:: console

  make clean

Building and releasing
======================

The release can be done from a containerized environment:

.. code-block:: console

  podman run --rm --workdir /io --entrypoint bash -it --volume `pwd`:/io:Z quay.io/pypa/manylinux2014_x86_64 -c "yum install -y make && make all"

To check what's happening, let's run a containerized environment - this can be
helpful when you are testing or developing the extension:

.. code-block:: console

  podman run --rm --workdir /io --entrypoint bash -it --volume `pwd`:/io:Z quay.io/pypa/manylinux2014_x86_64

The following commands (run in the container stated above) will install all
the necessary tools:

.. code-block:: console

  yum install -y make
  make deps

Once tests pass, clean the environment:

.. code-block:: console

  make clean

Now we should be ready to produce ``bdist_wheel`` and ``sdist`` distribution
for PyPI:

.. code-block:: console

  python3 setup.py bdist_wheel
  python3 setup.py sdist

Finally, upload artifacts to PyPI:

.. code-block:: console

  auditwheel repair fext/*.whl
  twine upload wheelhouse/*.whl

Alternativelly you can let ``make all`` happen.

Installation
============

The project is `hosted on PyPI <https://pypi.org/project/fext/>`_. You can
install it via ``pip`` or ``Pipenv``:

.. code-block:: console

  pipenv install fext
  # pip3 install fext

If there is no release conforming your system, a build process is triggered
during the installation - requires ``python3-devel`` and ``gcc/g++``.

Usage
=====

These data structures were designed for Thoth's adviser - for data kept in
resolver's internal state as well as in the reinforcement learning part.

