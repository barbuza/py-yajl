/*
 * Copyright 2009, R. Tyler Ballance <tyler@slide.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of R. Tyler Ballancenor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#include <Python.h>

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include "py_yajl.h"

static yajl_gen_status ProcessObject(_YajlEncoder *self, PyObject *object)
{
    yajl_gen handle = (yajl_gen)(self->_generator);
    yajl_gen_status status = yajl_gen_in_error_state;
    PyObject *iterator, *item;

    if (object == Py_None) {
        return yajl_gen_null(handle);
    }
    if (object == Py_True) {
        return yajl_gen_bool(handle, 1);
    }
    if (object == Py_False) {
        return yajl_gen_bool(handle, 0);
    }
    if (PyInt_Check(object)) {
        fprintf(stderr, "int\n");
        return yajl_gen_integer(handle, PyInt_AsLong(object));
    }
    if (PyLong_Check(object)) {
        return yajl_gen_integer(handle, PyLong_AsLong(object));
    }
    if (PyFloat_Check(object)) {
        return yajl_gen_double(handle, PyFloat_AsDouble(object));
    }
    if (PyList_Check(object)) {
        /*
         * Recurse and handle the list 
         */
        iterator = PyObject_GetIter(object);
        status = yajl_gen_array_open(handle);
        if (iterator == NULL)
            goto exit;
        while ((item = PyIter_Next(iterator))) {
            status = ProcessObject(self, item);
            Py_XDECREF(item);
        }
        Py_XDECREF(iterator);
        status = yajl_gen_array_close(handle);
        return status;
    }
    if (PyDict_Check(object)) {
    }
        
    exit:
        return yajl_gen_in_error_state;
}

PyObject *py_yajlencoder_encode(PYARGS)
{
    _YajlEncoder *encoder = (_YajlEncoder *)(self);
    PyObject *value, *result;
    yajl_gen generator = NULL;
    yajl_status yrc;
    const unsigned char *buffer;
    unsigned int buflen = 0;
    yajl_gen_config genconfig = { 0, NULL};
    yajl_gen_status status;

    if (!PyArg_ParseTuple(args, "O", &value))
        return NULL;
    
    generator = yajl_gen_alloc(&genconfig, NULL);
    encoder->_generator = generator;

    status = ProcessObject(encoder, value);

    if (status != yajl_gen_status_ok) {
        PyErr_SetObject(PyExc_ValueError, PyString_FromString("Failed to process"));
        return NULL;
    }

    yrc = yajl_gen_get_buf(generator, &buffer, &buflen);
    result = PyString_FromStringAndSize((const char *)(buffer), buflen);

    yajl_gen_free(generator);
    encoder->_generator = NULL;

    if ( (!buffer) || (!buflen) )
        return NULL;
    return result;
}

int yajlencoder_init(PYARGS)
{
    _YajlEncoder *me = (_YajlEncoder *)(self);

    if (!me)
        return 1;

    return 0;
}