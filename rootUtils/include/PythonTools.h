#ifndef _PYTHON_TOOLS_
#define _PYTHON_TOOLS_

#ifdef HAVE_PYTHON

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <vector>
#include <iostream>

std::vector<double> listTupleToVector_Double(PyObject* incoming);
std::vector<int> listTupleToVector_Int(PyObject* incoming);
std::vector<std::string> listTupleToVector_String(PyObject* incoming);

#endif
#endif