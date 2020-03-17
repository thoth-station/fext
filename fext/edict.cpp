/*
 * edict - An extended implementation of Python's dict.
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
/*
 * This module is based on the heapq implementation as present in the
 * Python standard library - git version used as a base for this
 * implementation: 1b55b65638254aa78b005fbf0b71fb02499f1852.
 *
 * This module adds an optimization for random item removal. Instead of
 * O(N) + O(log(N)) (item lookup and heap adjustment), the removal is
 * performed in O(log(N)). This speedup is significant for large Ns.
 */

#define PY_SSIZE_T_CLEAN

extern "C" {
#include <Python.h>
#include "structmember.h"
}

#include <iostream>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common.hpp"
#include "edict.hpp"
#include "eheapq.hpp"

typedef struct {
  PyObject_HEAD EDict<PyObject *, PyObject *, PyObjectRichCmpPairLT> *dict;
} ExtDict;

static int ExtDict_traverse(ExtDict *self, visitproc visit, void *arg) {
  for (auto it : (*self->dict)) {
    Py_VISIT(it.first);
    Py_VISIT(it.second);
  }

  return 0;
}

static int ExtDict_clear(ExtDict *self) {
  for (auto it : (*self->dict)) {
    Py_DECREF(it.first);
    Py_DECREF(it.second);
  }
  self->dict->clear();
  return 0;
}

static void ExtDict_dealloc(ExtDict *self) {
  PyObject_GC_UnTrack(self);
  ExtDict_clear(self);
  delete self->dict;
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtDict_new(PyTypeObject *type, PyObject *args,
                             PyObject *kwds) {
  ExtDict *self;
  self = (ExtDict *)type->tp_alloc(type, 0);
  self->dict = new EDict<PyObject *, PyObject *, PyObjectRichCmpPairLT>();
  return (PyObject *)self;
}

static int ExtDict_init(ExtDict *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"size", NULL};

  size_t size = self->dict->get_size();

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|k", kwlist, &size))
    return -1;

  self->dict->set_size(size);
  return 0;
}

static void added_callback(PyObject *key_added, PyObject *value_added) {
  Py_INCREF(key_added);
  Py_INCREF(value_added);
}

static void removed_callback(PyObject *key_removed, PyObject *value_removed) {
  Py_DECREF(key_removed);
  Py_DECREF(value_removed);
}

int ExtDict_setitem(ExtDict *self, PyObject *key, PyObject *value) {
  PyObject *result;

  if (value == NULL) {
    try {
      result = self->dict->remove(key);
      Py_DECREF(key);
      Py_DECREF(result);
    } catch (EDictKeyError &exc) {
      PyErr_SetString(PyExc_KeyError, exc.what());
      return -1;
    }
    return 0;
  }

  try {
    self->dict->set_or_replace(key, value, added_callback, removed_callback);
  } catch (ObjCmpErr &exc) {
    return -1;
  }

  return 0;
}

PyObject *ExtDict_getitem(ExtDict *self, PyObject *key) {
  PyObject *result;

  try {
    result = self->dict->get(key);
  } catch (EDictKeyError &exc) {
    PyErr_SetString(PyExc_KeyError, exc.what());
    return NULL;
  }

  Py_INCREF(result);
  return result;
}

PyObject *ExtDict_dict_clear(ExtDict *self) {
  ExtDict_clear(self);
  Py_RETURN_NONE;
}

PyObject *ExtDict_get(ExtDict *self, PyObject *args) {
  PyObject *key;
  PyObject *result;

  if (!PyArg_ParseTuple(args, "O", &key))
    return NULL;

  try {
    result = self->dict->get(key);
  } catch (EDictKeyError &exc) {
    Py_RETURN_NONE;
  }

  Py_INCREF(result);
  return result;
}

PyObject *ExtDict_items(ExtDict *self) {
  PyObject *result = PyList_New(self->dict->get_length());
  PyObject *item;

  int i = 0;
  for (auto it = self->dict->begin(); it != self->dict->end(); ++it, ++i) {
    item = PyTuple_New(2);
    Py_INCREF(it->first);
    Py_INCREF(it->second);
    PyTuple_SET_ITEM(item, 0, it->first);
    PyTuple_SET_ITEM(item, 1, it->second);
    PyList_SET_ITEM(result, i, item);
  }

  return result;
}

PyObject *ExtDict_keys(ExtDict *self) {
  PyObject *result = PyList_New(self->dict->get_length());

  int i = 0;
  for (auto it = self->dict->begin(); it != self->dict->end(); ++it, ++i) {
    Py_INCREF(it->first);
    PyList_SET_ITEM(result, i, it->first);
  }

  return result;
}

PyObject *ExtDict_values(ExtDict *self) {
  PyObject *result = PyList_New(self->dict->get_length());

  int i = 0;
  for (auto it = self->dict->begin(); it != self->dict->end(); ++it, ++i) {
    Py_INCREF(it->second);
    PyList_SET_ITEM(result, i, it->second);
  }

  return result;
}

static PyObject *ExtDict_getsize(ExtDict *self) {
  return PyLong_FromUnsignedLong(self->dict->get_size());
}

static long int ExtDict_len(ExtDict *self) { return self->dict->get_length(); }

static PyMethodDef ExtDict_methods[] = {
    {"clear", (PyCFunction)ExtDict_dict_clear, METH_VARARGS,
     "Clear the dictionary."},
    {"get", (PyCFunction)ExtDict_get, METH_VARARGS,
     "Get an item from the dict; return None if not present."},
    {"items", (PyCFunction)ExtDict_items, METH_VARARGS,
     "Return a list containing a tuple for each key value pair."},
    {"keys", (PyCFunction)ExtDict_keys, METH_VARARGS,
     "Return a list containing the dictionary's keys."},
    {"values", (PyCFunction)ExtDict_values, METH_VARARGS,
     "	Return a list of all the values in the dictionary."},
    {NULL}};

static PyMappingMethods ExtDict_mapping_methods[] = {
    (lenfunc)ExtDict_len,           // mp_length
    (binaryfunc)ExtDict_getitem,    // mp_subscript
    (objobjargproc)ExtDict_setitem, // mp_ass_subscript
    {NULL}};

static PyGetSetDef ExtDict_getsetters[] = {
    {"size", (getter)ExtDict_getsize, NULL, "Max size of the dictionary.",
     NULL},
    {NULL},
};

PyMODINIT_FUNC PyInit_edict(void) {
  static PyTypeObject ExtDict = {PyVarObject_HEAD_INIT(NULL, 0)};
  ExtDict.tp_name = "eheapq.ExtDict";
  ExtDict.tp_doc = "Extended heap queue algorithm.";
  ExtDict.tp_basicsize = sizeof(ExtDict);
  ExtDict.tp_itemsize = 0;
  ExtDict.tp_flags =
      Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC;
  ExtDict.tp_new = ExtDict_new;
  ExtDict.tp_init = (initproc)ExtDict_init;
  ExtDict.tp_dealloc = (destructor)ExtDict_dealloc;
  ExtDict.tp_traverse = (traverseproc)ExtDict_traverse;
  ExtDict.tp_clear = (inquiry)ExtDict_clear;
  ExtDict.tp_methods = ExtDict_methods;
  ExtDict.tp_getset = ExtDict_getsetters;
  ExtDict.tp_as_mapping = ExtDict_mapping_methods;

  static PyModuleDef eheapq = {PyModuleDef_HEAD_INIT};
  eheapq.m_name = "edict";
  eheapq.m_doc = "Implementation of extended dictionary.";
  eheapq.m_size = -1;

  PyObject *m;
  if (PyType_Ready(&ExtDict) < 0)
    return NULL;

  m = PyModule_Create(&eheapq);
  if (!m)
    return NULL;

  Py_INCREF(&ExtDict);
  if (PyModule_AddObject(m, "ExtDict", (PyObject *)&ExtDict) < 0) {
    Py_DECREF(&ExtDict);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
