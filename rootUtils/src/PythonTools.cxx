
#include "PythonTools.h"
#ifdef HAVE_PYTHON


std::vector<double> listTupleToVector_Double(PyObject* incoming) {
   std::vector<double> data;
   if (PyTuple_Check(incoming)) {
      for(Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
         PyObject *value = PyTuple_GetItem(incoming, i);
         data.push_back( PyFloat_AsDouble(value) );
      }
   } else {
      if (PyList_Check(incoming)) {
         for(Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
            PyObject *value = PyList_GetItem(incoming, i);
            data.push_back( PyFloat_AsDouble(value) );
         }
      } else {
         //throw logic_error("Passed PyObject pointer was not a list or tuple!");
         std::cout <<"Passed PyObject pointer was not a list or tuple!" <<std::endl;
      }
   }
   return data;
}

std::vector<int> listTupleToVector_Int(PyObject* incoming) {
   std::vector<int> data;
   if (PyTuple_Check(incoming)) {
      for(Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
         PyObject *value = PyTuple_GetItem(incoming, i);
         data.push_back( PyFloat_AsDouble(value) );
      }
   } else {
      if (PyList_Check(incoming)) {
         for(Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
            PyObject *value = PyList_GetItem(incoming, i);
            data.push_back( PyFloat_AsDouble(value) );
         }
      } else {
         //throw logic_error("Passed PyObject pointer was not a list or tuple!");
         std::cout<<"Passed PyObject pointer was not a list or tuple!"<<std::endl;;
      }
   }
   return data;
}

std::vector<std::string> listTupleToVector_String(PyObject* incoming) {
   std::vector<std::string> data;
   if (PyTuple_Check(incoming)) {
      for(Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
         PyObject *value = PyTuple_GetItem(incoming, i);
#if PY_MAJOR_VERSION >= 3
         data.push_back( PyUnicode_AsUTF8(value) );
#else
         data.push_back( PyString_AsString(value) );
#endif
      }
   } else {
      if (PyList_Check(incoming)) {
         for(Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
            PyObject *value = PyList_GetItem(incoming, i);
            //data.push_back( PyUnicode_AsUTF8(value) );
#if PY_MAJOR_VERSION >= 3
            data.push_back( PyUnicode_AsUTF8(value) );
#else
            data.push_back( PyString_AsString(value) );
#endif
         }
      } else {
         //throw logic_error("Passed PyObject pointer was not a list or tuple!");
         std::cout<<"Passed PyObject pointer was not a list or tuple!"<<std::endl;
      }
   }
   return data;
}


#endif