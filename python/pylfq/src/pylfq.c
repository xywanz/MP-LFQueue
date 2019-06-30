#include "pylfq.h"
#include <structmember.h>

struct ModuleState {
    PyObject *error;
};

static PyObject *PyLFQ_Create(PyObject *self, PyObject *args)
{
        int key;
        long size;
        uint32_t count;
        PyObject *overwrite = Py_False;

        if (!PyArg_ParseTuple(args, "ilI|O", &key, &size, &count, &overwrite))
                return Py_False;

        if (LFQueue_create(key, size, count, overwrite == Py_False? false : true) != 0) {
                PyErr_SetString(PyExc_RuntimeError, "Error occured in LFQueue_create");
                return Py_False;
        }

        return Py_True;
}

static PyObject *PyLFQ_Destory(PyObject *self, PyObject *args)
{
        int key;

        if (!PyArg_ParseTuple(args, "i", &key))
                return Py_False;

        if (LFQueue_destroy(key) != 0)
                return Py_False;

        return Py_True;
}

static int PyLFQ_Init(PyLFQ *self, PyObject *args, PyObject *kwdict)
{
        static char *kwlist[] = {"key", NULL};
        int key = -1;

        if (!PyArg_ParseTupleAndKeywords(args, kwdict, "i|", kwlist, &key))
                return -1;

        if ((self->queue = LFQueue_open(key)) == NULL) {
                PyErr_SetString(PyExc_RuntimeError, "Error occured in LFQueue_open");
                return -1;
        }

        return 0;
}

static void PyLFQ_Dealloc(PyLFQ *self)
{
        LFQueue_close(self->queue);
        Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyLFQ_Push(PyLFQ *self, PyObject *args)
{
        int64_t seq;
        int64_t input_size = -1;
        uint64_t size;
        LFQueue *queue;
        PyBytesObject *bytes;

        if (!PyArg_ParseTuple(args, "S|l", &bytes, &input_size))
                return Py_BuildValue("l", -1L);

        if (!bytes)
                return Py_BuildValue("l", -1L);
        
        size = input_size;
        if (size < 0)
                size = PyBytes_GET_SIZE(bytes);

        queue = self->queue;
        if (size > queue->header->node_data_size)
                return Py_BuildValue("l", -1L);

        seq = LFQueue_push(self->queue, PyBytes_AsString((PyObject *)bytes), size);
        return Py_BuildValue("l", seq);
}

static PyObject *PyLFQ_Pop(PyLFQ *self, PyObject *args)
{
        int64_t seq = -1;
        uint64_t size;
        LFQueue *queue;
        PyBytesObject *bytes;

        if (!PyArg_ParseTuple(args, "S", &bytes))
                return Py_BuildValue("l", -1L);

        if (!bytes)
                return Py_BuildValue("l", -1L);

        queue = self->queue;
        size = PyBytes_GET_SIZE(bytes);
        if (size < queue->header->node_data_size)
                return Py_BuildValue("l", -1L);

        seq = LFQueue_pop(self->queue, PyBytes_AsString((PyObject *)bytes), &size);
        return Py_BuildValue("(l,l)", seq, size);
}

static PyMemberDef PyLFQ_members[] = {
        {NULL}
};

static PyMethodDef PyLFQ_methods[] = {
        {"push", (PyCFunction)PyLFQ_Push, METH_VARARGS, "pylfq push"},
        {"pop", (PyCFunction)PyLFQ_Pop, METH_VARARGS, "pylfq pop"},
        {NULL}
};

static PyMethodDef module_methods[] = {
        {"create", (PyCFunction)PyLFQ_Create, METH_VARARGS, "pylfq create"},
        {"destroy", (PyCFunction)PyLFQ_Destory, METH_VARARGS, "pylfq destroy"},
        {NULL}
};

static PyTypeObject PyLFQ_type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        "lfq.PyLFQ",    /* tp_name: For printing, in format "<module>.<name>" */
        sizeof(PyLFQ),  /* tp_basicsize */
        0,              /* tp_itemsize: For allocation */
        (destructor)PyLFQ_Dealloc,  /* tp_dealloc */
        0,              /* tp_print */
        0,              /* tp_getattr */
        0,              /* tp_setattr */
        0,              /* tp_as_async: formerly known as tp_compare (Python 2) */
        0L,             /* tp_repr */
        0,              /* tp_as_number */
        0,              /* tp_as_sequence */
        0,              /* tp_as_mapping */
        0,              /* tp_hash */
        0,              /* tp_call */
        0,              /* tp_str */
        0,              /* tp_getattro */
        0,              /* tp_setattro */
        0,              /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        "PyLFQ",        /* tp_doc: Documentation string */
        0,              /* tp_traverse */
        0,              /* tp_clear */
        0,              /* tp_richcompare */
        0,              /* tp_weaklistoffset */
        0,              /* tp_iter */
        0,              /* tp_iternext */
        PyLFQ_methods,  /* tp_methods */
        PyLFQ_members,  /* tp_members */
        0,              /* tp_getset */
        0,              /* tp_base */
        0,              /* tp_dict */
        0,              /* tp_descr_get */
        0,              /* tp_descr_set */
        0,              /* tp_dictoffset */
        (initproc)PyLFQ_Init, /* tp_init */
        0,              /* tp_alloc */
        PyType_GenericNew,  /* tp_new */
        0,              /* tp_free: Low-level free-memory routine */
        0,              /* tp_is_gc: For PyObject_IS_GC */
        0,              /* tp_bases */
        0,              /* tp_mro: method resolution order */
        0,              /* tp_cache */
        0,              /* tp_subclasses */
        0,              /* tp_weaklist */
        0,              /* tp_del */
        0,              /* tp_version_tag */
        0,              /* tp_finalize */
};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "pylfq",
        NULL,
        sizeof(struct ModuleState),
        module_methods,
        NULL,
        NULL,
        NULL,
        NULL
};

PyMODINIT_FUNC PyInit_pylfq(void)
{
        PyObject *module;
        struct ModuleState *st;

        if (PyType_Ready(&PyLFQ_type) != 0)
                return NULL;

        module = PyModule_Create(&moduledef);
        if (module == NULL)
                return NULL;

        st = (struct ModuleState*)PyModule_GetState(module);
        st->error = PyErr_NewException("pylfq.Error", NULL, NULL);
        if (st->error == NULL) {
                Py_DECREF(module);
                return NULL;
        }

        PyModule_AddObject(module, "MQ", (PyObject *)&PyLFQ_type);
        Py_INCREF(&PyLFQ_type);
        return module;
}
