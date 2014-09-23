#include <Python.h>

#include "munkres.h"
#include <iostream>

#ifndef PyInt_Check
#define PyInt_Check(x) PyLong_Check(x)
#endif
#ifndef PyInt_AsLong
#define PyInt_AsLong(x) PyLong_AsLong(x)
#endif

using namespace std;

bool marshal_pyargs_to_entries(PyObject *self, PyObject *args, vector<entry> &entries, bool &with_cost, bool &with_idx) {
    PyObject *py_entries_arg;
    PyObject *py_entries;
    PyObject *py_entry_seq;    
    PyObject *py_entry;
    PyObject *value;
    int i, len, slen;
    int with_cost_int = 0;
    int with_idx_int = 0;
    
    char* wrong_arg_msg = "Expected a sequence of (i,j,cost)";

    /* Marshal the list of tuples in *args into the entries vector */
    if (!PyArg_ParseTuple(args, "O|ii", &py_entries_arg,&with_cost_int,&with_idx_int))
        return false;
    
    with_cost = (with_cost_int != 0);
    with_idx = (with_idx_int != 0);

    py_entries = PySequence_Fast(py_entries_arg, wrong_arg_msg);
    if (!py_entries) {
        PyErr_SetString(PyExc_ValueError, wrong_arg_msg);
        return false;
    }
    len = PySequence_Size(py_entries);
    for (i = 0; i < len; i++) {
        py_entry = PySequence_Fast_GET_ITEM(py_entries, i);
        entry e;
        e.idx = i;
        py_entry_seq = PySequence_Fast(py_entry, wrong_arg_msg);
        if (!py_entry_seq) {
            PyErr_SetString(PyExc_ValueError, wrong_arg_msg);
            return false;
        }
        slen = PySequence_Size(py_entry_seq);
        if (slen != 3) {
            PyErr_SetString(PyExc_ValueError, wrong_arg_msg);
            return false;
        }
        // todo: check that slen == 3.
        value = PySequence_Fast_GET_ITEM(py_entry_seq, 0);
        if (!PyInt_Check(value)) { PyErr_SetString(PyExc_TypeError, "Matrix indices must be integer."); return false; }
        e.pos.i = PyInt_AsLong(value);
        value = PySequence_Fast_GET_ITEM(py_entry_seq, 1);
        if (!PyInt_Check(value)) { PyErr_SetString(PyExc_TypeError, "Matrix indices must be integer."); return false; }
        e.pos.j = PyInt_AsLong(value);
        value = PySequence_Fast_GET_ITEM(py_entry_seq, 2);
        //if (!PyFloat_Check(value)) { PyErr_SetString(PyExc_TypeError, "Costs must be float"); return NULL; }
        e.cost = PyFloat_AsDouble(value);
        if (e.cost == -1.0 && PyErr_Occurred()) {
            PyErr_SetString(PyExc_TypeError, "Costs must be float"); 
            return false; 
        }
        
        entries.push_back(e);
                
        Py_DECREF(py_entry_seq);        
        /* DON'T DECREF item here */
    }
    Py_DECREF(py_entries);
    return true;
}

PyObject* marshal_entries_to_list(vector<entry> &entries, bool with_cost, bool with_idx) {
    // Marshal the entries back to a python list of tuples.
    PyObject *py_result = PyList_New(entries.size());
    for (uint i = 0; i < entries.size(); i++) {
        entry &e = entries[i];
        PyObject *res_entry;
        if (with_cost && with_idx) {
            res_entry = Py_BuildValue("llld", e.idx, e.pos.i, e.pos.j, e.cost);
        }
        else if (with_cost) {
            res_entry = Py_BuildValue("lld", e.pos.i, e.pos.j, e.cost);
        } 
        else if (with_idx) {
            res_entry = Py_BuildValue("lll", e.idx, e.pos.i, e.pos.j);
        }
        else {
            res_entry = Py_BuildValue("ll", e.pos.i, e.pos.j);
        }
        // SetItem steals the reference to res_entry, so we don't have to DECREF it.
        PyList_SetItem(py_result, i, res_entry);
    }
    
    return py_result;
}

static PyObject *
munkres_munkres(PyObject *self, PyObject *args)
{
    vector<entry> entries;
    bool with_cost;
    bool with_idx;
    if (!marshal_pyargs_to_entries(self, args, entries, with_cost, with_idx)) {
        return NULL;
    }
    vector<entry> optimal = munkres(entries);
    PyObject *py_result = marshal_entries_to_list(optimal, with_cost, with_idx);
    return py_result;
}

char* munkres_fnc_doc = "munkres([(i,j,cost)...], return_costs=False, return_idx=False).\n"
                    "Returns the optimal matching as a list of tuples [(idx, i, j, cost)...].\n"
                    "Both idx and cost are excluded from the return tuples by default "
                    "(return_costs and return_idx)"
                    "";
  
static PyMethodDef MunkresMethods[] = {
    {"munkres", munkres_munkres, METH_VARARGS, munkres_fnc_doc},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

char* munkres_doc = "Implementation of the sparse kuhn-munkres algorithm."
    " For a general description of the full munkres algorithm, see"
    " http://csclab.murraystate.edu/bob.pilgrim/445/munkres.html."
    " This implementation assumes a non-complete graph and stores"
    " the elements in a sparse matrix, following the ideas outlined"
    " in http://dl.acm.org/citation.cfm?id=1388969.1389035"
    " (Sailor Assignment Problem)";

#if PY_VERSION_HEX >= 0x03000000

/* Python 3.x code */

static struct PyModuleDef munkresmodule = {
   PyModuleDef_HEAD_INIT,
   "csparsemunkres", /* name of module */
   munkres_doc, /* module documentation, may be NULL */
   -1, /* size of per-interpreter state of the module,
or -1 if the module keeps state in global variables. */
   MunkresMethods
};

PyMODINIT_FUNC
PyInit_csparsemunkres(void)
{
    (void) PyModule_Create(&munkresmodule);
}

#else

/* Python 2.x code */

PyMODINIT_FUNC
initcsparsemunkres(void)
{
    (void) Py_InitModule3("csparsemunkres", MunkresMethods, munkres_doc);
}

#endif
