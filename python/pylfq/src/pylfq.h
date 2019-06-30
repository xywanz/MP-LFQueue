#ifndef _LOCKFREEQUEUE_PYLFQ_H_
#define _LOCKFREEQUEUE_PYLFQ_H_

#include <Python.h>
#include "queue.h"

typedef struct PyLFQ
{
    PyObject_HEAD

    LFQueue *queue;
} PyLFQ;

#endif
