/******************************************************************************
 * Wrapper for the Python Dijsktra API.
 */

#include <C:/Python27/include/Python.h>

#include "dijkstra.h"

// Docstrings
static char module_docstring[] =
"This module provides a fast C implementation of Dijkstra's path-finding algorithm.";
static char dijkstra_docstring[] =
"This function accepts a list of connections with ends specified as integer identifications with a float cost. "
"It assembles them into a network and finds the path of minimum cost from the specified start to the specified end.";

// Method declarations
static PyObject * dijkstra_Dijkstra(PyObject * self, PyObject * args);

// Method table
static PyMethodDef module_methods[] = {
	{"Dijkstra", dijkstra_Dijkstra, METH_VARARGS, dijkstra_docstring},
	{NULL, NULL, 0, NULL}
};

// Init function
PyMODINIT_FUNC init_dijkstra(void) {
	PyObject * m = Py_InitModule3("_dijkstra", module_methods, module_docstring);
	if (m == NULL) return;
}

// Method definitions
static PyObject * dijkstra_Dijkstra(PyObject * self, PyObject * args) {
	
	// Parse arguments
	PyObject * arg_list = NULL;
	unsigned int start;
	unsigned int end;
	PyObject * arg_twoway;

	if (!PyArg_ParseTuple(args, "OIIO!", &arg_list, &start, &end, &PyBool_Type, &arg_twoway)) {
		PyErr_SetString(PyExc_TypeError, "Parameters must be a list of (int, int, float) tuples, two ints, and a bool");
		return NULL;
	}

	PyObject * sequence = PySequence_Fast(arg_list, "Expected a sequence");
	Py_ssize_t len = PySequence_Size(sequence);

	// Build array from list
	Connection * cons = (Connection *)malloc(len * sizeof(Connection));
	for (int i = 0; i < len; i++) {
		PyObject * list_item = PySequence_Fast_GET_ITEM(arg_list, i);
		// Check for tuple type
		if (!PyTuple_Check(list_item)) {
			PyErr_SetString(PyExc_TypeError, "Connection must be an int-int-float tuple");
			return NULL;
		}
		// Extract data from tuple
		Connection * con = cons + i;
		if (!PyArg_ParseTuple(list_item, "IIf", &con->start, &con->end, &con->cost)) {
			PyErr_SetString(PyExc_TypeError, "Connection must be an int-int-float tuple");
			return NULL;
		}
	}
	Py_DECREF(sequence);

	// Add two-way connections
	int twoway = PyObject_IsTrue(arg_twoway);
	if (twoway) {
		Connection * temp = Connection_TwoWay(cons, len);
		free(cons);
		cons = temp;
		len = len * 2;
	}

	// Run the Dijktra algorithm
	DijkstraOutput results = Dijkstra_ShortestPath(cons, len, start, end);

	// Check for Dijkstra errors
	switch (results.error) {
	case DIJKSTRA_SUCCESS:
		break;
	case DIJKSTRA_ERROR_NO_CONNECTIONS:
		PyErr_SetString(PyExc_ValueError, "No connections given");
		return NULL;
	case DIJKSTRA_ERROR_NEGATIVE_COSTS:
		PyErr_SetString(PyExc_ValueError, "There cannot be negative costs");
		return NULL;
	case DIJKSTRA_ERROR_MISSING_NODE_IDS:
		PyErr_SetString(PyExc_ValueError, "Node id numbers must be consecutive");
		return NULL;
	case DIJKSTRA_ERROR_INVALID_START:
		PyErr_SetString(PyExc_ValueError, "Start node is not included in the data");
		return NULL;
	case DIJKSTRA_ERROR_INVALID_END:
		PyErr_SetString(PyExc_ValueError, "End node is not included in the data");
		return NULL;
	case DIJKSTRA_ERROR_START_AND_END_NOT_CONNECTED:
		PyErr_SetString(PyExc_ValueError, "The network does not have the start and end nodes connected");
		return NULL;
		/*
	default:
		PyErr_SetString(PyExc_StandardError, "Unknown error");
		return NULL;
		*/
	}

	// Assemble output
	PyObject * output_list = PyList_New(results.n_elements);
	for (unsigned int i = 0; i < results.n_elements; i++) {
		
		PyList_SetItem(output_list, i, PyInt_FromLong(results.path[i]));
	}
	PyObject * py_output = Py_BuildValue("(Of)", output_list, results.total_cost);
	return py_output;
}