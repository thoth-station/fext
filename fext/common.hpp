/*
 * common - Shared logic for fext.
 * Copyright(C) 2020 Fridolin Pokorny
 *
 * This program is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define PY_SSIZE_T_CLEAN

#include <utility>

extern "C" {
#include "structmember.h"
#include <Python.h>
}

/**
 * The exception raised if the Python objects cannot be compared.
 */
class ObjCmpErr : public std::exception {
public:
  virtual const char *what() const throw() {
    return "failed to compare Python objects";
  }
} ObjCmpErrExc;

/**
 * Function object type implementing the rich object comparision on Python objects.
 */
struct PyObjectRichCmpLT {
  /**
   * Compare (rich compare) two Python objects, return the lesser one.
   *
   * @raises ObjCmpErr If objects cannot be compared.
   */
  bool operator()(PyObject *a, PyObject *b) {
    Py_INCREF(a);
    Py_INCREF(b);
    auto cmp = PyObject_RichCompareBool(a, b, Py_LT);
    Py_DECREF(a);
    Py_DECREF(b);

    if (cmp < 0)
      throw ObjCmpErrExc;

    return bool(cmp);
  }
};

/**
 * Function object type implementing the rich object comparision
 * on Python objects, Python objects are stored as pairs and
 * the comparision is done on the second.
 */
struct PyObjectRichCmpPairLT {
  /**
   * Compare (rich compare) two Python objects, return the lesser one.
   *
   * @raises ObjCmpErr If objects cannot be compared.
   */
  bool operator()(std::pair<PyObject *, PyObject *> & a, std::pair<PyObject *, PyObject *> & b) {
    Py_INCREF(a.second);
    Py_INCREF(b.second);
    auto cmp = PyObject_RichCompareBool(a.second, b.second, Py_LT);
    Py_DECREF(a.second);
    Py_DECREF(b.second);

    if (cmp < 0)
      throw ObjCmpErrExc;

    return bool(cmp);
  }
};

