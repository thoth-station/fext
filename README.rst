fext
----

Fast CPython extensions to Python standard library with focus on performance.

This library provides CPython native extensions to mimic some of the well known
built-in types. The implementation relies on enforced protocol - all the
objects and abstract data types are implemented in C/C++ to provide highly
effective manipulation.

Extended dict - fext.edict.ExtDict
==================================

The implementation of ``fext.ExtDict`` was introduce to provide a data
structure capable of limiting number of items stored in a dictionary (a hash
table).

.. figure:: https://raw.githubusercontent.com/thoth-station/fext/master/fig/fext_extdict.png
   :align: center

``fext.ExtDict`` is implemented as a standard Python ``dict`` and supports most
``dict`` methods. To restrict number of items stored in the dictionary,
``fext.edict.ExtDict`` uses a min-heap queue that stores key-value pairs (key
to the dictionary with it's value). When an item is inserted into the
dictionary, ``fext.ExtDict`` checks its size and possibly performs
``pushpop``-like operation on the heap to limit number of items stored.

The comparision is done solely on the actual value. Keys are stored in min-heap
queue together with values as pairs to guarantee O(1) time when removing an
item from the ``fext.edictExtDict``.

Extended heapq - fext.edict.ExtHeapQueue
========================================

The extended heap queue acts as a min-heap queue from the standard Python
library.  It uses a hash table for storing information about indexes (where
values sit in the min-heap queue) to optimize removals from the heap.

.. figure:: https://raw.githubusercontent.com/thoth-station/fext/master/fig/fext_extheapq.png
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

Reference count and memory leak checks
======================================

You can find Makefile in the Git repo. This repo defines targets to perform
leak checks and reference count checks. Note they use different Python
interpreters so make sure you do not mix virtual environments when running the
tests.

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

Mind ``make-refcount`` and ``make check-leaks`` will take some time given the
checks and processing that is done on the background. To verify your changes
more iterativelly, ``make test`` should do the trick (don't forget to do ``make
check`` after that though).

To clean up your environment, perform:

.. code-block:: console

  make clean

Building and releasing
======================

First, let's run a containerized environment (make sure you are in the root of
this repo):

.. code-block:: console

  podman run --rm --entrypoint bash -it --volume ./:/home:Z centos:8

The following commands (run in the container stated above) will install all
the necessary tools:

.. code-block:: console

  dnf install -y make
  make deps

Once tests pass, clean the environment:

.. code-block:: console

  make clean

Now we should be ready to produce ``bdist`` and ``sdist`` distribution for PyPI:

.. code-block:: console

  python3 setup.py bdist
  python3 setup.py sdist

Finally, upload artifacts to PyPI:

.. code-block:: console

  twine upload dist/*

