/*a Copyright
  
  This file 'python_filter.cpp' copyright Gavin J Stark 2016
  
  This is free software; you can redistribute it and/or modify it however you wish,
  with no obligations
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*a Includes
 */
#include <Python.h>
#include "python_quaternion.h"
#include "python_lens_projection.h"
#include "lens_projection.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_lens_projection
 */
typedef struct {
    PyObject_HEAD
    c_lens_projection *lens_projection;
} t_PyObject_lens_projection;


/*a Forward function declarations
 */
static int       python_lens_projection_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *python_lens_projection_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void      python_lens_projection_dealloc(PyObject *self);
static PyObject *python_lens_projection_getattr(PyObject *self, char *attr);
static int       python_lens_projection_setattr(PyObject *o, char *attr_name, PyObject *v);
static PyObject *python_lens_projection_str(PyObject *self);

static PyObject *python_lens_projection_class_method_add_named_polynomial(PyObject* cls, PyObject* args, PyObject *kwds);

static PyObject *python_lens_projection_method_set_lens(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_lens_projection_method_set_sensor(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_lens_projection_method_set_polynomial(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_lens_projection_method_orient(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_lens_projection_method_orientation_of_xy(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_lens_projection_method_xy_of_orientation(PyObject* self, PyObject* args, PyObject *kwds);

/*a Static variables
 */
/*v python_lens_projection_methods
 */
PyMethodDef python_lens_projection_methods[] = {
    {"add_named_polynomial",     (PyCFunction)python_lens_projection_class_method_add_named_polynomial,           METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"set_lens",    (PyCFunction)python_lens_projection_method_set_lens,   METH_VARARGS|METH_KEYWORDS},
    {"set_sensor",  (PyCFunction)python_lens_projection_method_set_sensor, METH_VARARGS|METH_KEYWORDS},
    {"set_polynomial",    (PyCFunction)python_lens_projection_method_set_polynomial,   METH_VARARGS|METH_KEYWORDS},
    {"orient",      (PyCFunction)python_lens_projection_method_orient,     METH_VARARGS|METH_KEYWORDS},
    {"orientation_of_xy",  (PyCFunction)python_lens_projection_method_orientation_of_xy,     METH_VARARGS|METH_KEYWORDS},
    {"xy_of_orientation",  (PyCFunction)python_lens_projection_method_xy_of_orientation,     METH_VARARGS|METH_KEYWORDS},
    {NULL, NULL},
};

/*v PyTypeObject_lens_projection
 */
static PyTypeObject PyTypeObject_lens_projection = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "lens_projection", // type name
    sizeof(t_PyObject_lens_projection), // basic size
    0, // item size - zero for static sized object types
    python_lens_projection_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_lens_projection_getattr, /*tp_getattr*/
    python_lens_projection_setattr, /*tp_setattr*/
    0, /*tp_compare*/
    0, /*tp_repr - ideally a represenation that is python that recreates this object */
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash */
	0, /* tp_call - called if the object itself is invoked as a method */
	python_lens_projection_str, /* tp_str */
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Lens_Projection object",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_lens_projection_methods, /* tp_methods */
    0, //python_quaternion_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_lens_projection_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_lens_projection_new,     /* tp_new */
};


/*f python_lens_projection_class_method_add_named_polynomial
 */
static PyObject *
python_lens_projection_class_method_add_named_polynomial(PyObject* cls, PyObject* args, PyObject *kwds)
{
    const char *name;
    PyObject *poly, *inv_poly;

    static const char *kwlist[] = {"name", "poly", "inv_poly", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sOO", (char **)kwlist, 
                                     &name, &poly, &inv_poly))
        return NULL;

#define MAX_COEFFS 20
    double poly_coeffs[MAX_COEFFS];
    double inv_poly_coeffs[MAX_COEFFS];
    PyObject *seq = PySequence_Fast(poly, "Polynomials be a sequence of doubles");
    int poly_len = PySequence_Size(seq);
    for (int i=0; (i<poly_len) && (i<MAX_COEFFS); i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(seq, i);
        poly_coeffs[i] = PyFloat_AsDouble(item);
    }
    Py_DECREF(seq);
    seq = PySequence_Fast(inv_poly, "Polynomials be a sequence of doubles");
    int inv_poly_len = PySequence_Size(seq);
    for (int i=0; (i<inv_poly_len) && (i<MAX_COEFFS); i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(seq, i);
        inv_poly_coeffs[i] = PyFloat_AsDouble(item);
    }
    Py_DECREF(seq);
    fprintf(stderr,"%d:%d\n",poly_len,inv_poly_len);
    if (!PyErr_Occurred()) {
        return Py_BuildValue("i",c_lens_projection::add_named_polynomial(name, poly_len, poly_coeffs, inv_poly_len, inv_poly_coeffs));
    }
    return NULL;
}

/*a Python lens_projection object methods
 */
/*f python_lens_projection_method_set_lens
 */
static PyObject *
python_lens_projection_method_set_lens(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    double focal_length, frame_width;
    const char *lens_type;
    static const char *kwlist[] = {"focal_length", "frame_width", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "dds", (char **)kwlist, 
                                     &focal_length, &frame_width, &lens_type))
        return NULL;

    if (py_obj->lens_projection) {
        if (!py_obj->lens_projection->set_polynomial(lens_type)) {
            t_lens_projection_type lp_type = c_lens_projection::lens_projection_type("polynomial");
            py_obj->lens_projection->set_lens(frame_width, focal_length, lp_type);
            py_obj->lens_projection->set_polynomial(lens_type);
        } else {
            t_lens_projection_type lp_type = c_lens_projection::lens_projection_type(lens_type);
            py_obj->lens_projection->set_lens(frame_width, focal_length, lp_type);
        }
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_method_set_sensor
 */
static PyObject *
python_lens_projection_method_set_sensor(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    double width;
    double height=0.0;
    static const char *kwlist[] = {"width", "height", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d|d", (char **)kwlist, 
                                     &width, &height))
        return NULL;

    height = (height==0)?width:height;
    if (py_obj->lens_projection) {
        py_obj->lens_projection->set_sensor(width, height);
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_method_set_polynomial
 */
static PyObject *
python_lens_projection_method_set_polynomial(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    const char *name;
    static const char *kwlist[] = {"name", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)kwlist, 
                                     &name))
        return NULL;

    if (py_obj->lens_projection) {
        if (py_obj->lens_projection->set_polynomial(name)) {
            PyErr_SetString(PyExc_RuntimeError, "Unknown polynomial name");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_method_orient
 */
static PyObject *
python_lens_projection_method_orient(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    PyObject *orientation=NULL;
    static const char *kwlist[] = {"orientation", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, 
                                     &orientation))
        return NULL;

    if (py_obj->lens_projection) {
        c_quaternion *quaternion;
        if (python_quaternion_data(orientation, 0, (void *)&quaternion)) {
            py_obj->lens_projection->orient(quaternion->copy());
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Orientation must be a quaternion");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_method_orientation_of_xy
 */
static PyObject *
python_lens_projection_method_orientation_of_xy(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    PyObject *xy;
    double xy_d[2];

    static const char *kwlist[] = {"xy", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, 
                                     &xy))
        return NULL;

    if (!PyArg_ParseTuple(xy, "dd", &xy_d[0], &xy_d[1])) return NULL;

    if (py_obj->lens_projection) {
        c_quaternion *q = new c_quaternion();
        *q = py_obj->lens_projection->orientation_of_xy(xy_d);
        return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_method_xy_of_orientation
 */
static PyObject *
python_lens_projection_method_xy_of_orientation(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    PyObject *orientation=NULL;
    static const char *kwlist[] = {"orientation", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, 
                                     &orientation))
        return NULL;

    if (py_obj->lens_projection) {
        c_quaternion *quaternion;
        if (python_quaternion_data(orientation, 0, (void *)&quaternion)) {
            double xy[2];
            py_obj->lens_projection->xy_of_orientation(quaternion, xy);
            return Py_BuildValue("dd",xy[0],xy[1]);
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Orientation must be a quaternion");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_str
 */
static PyObject *
python_lens_projection_str(PyObject* self)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;
    
    if (py_obj->lens_projection) {
        char buffer[1024];
        py_obj->lens_projection->__str__(buffer, sizeof(buffer));
        return PyString_FromFormat("%s", buffer);
    }
    Py_RETURN_NONE;
}

/*f python_lens_projection_dealloc
 */
static void
python_lens_projection_dealloc(PyObject *self)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;
    if (py_obj->lens_projection) {
        delete(py_obj->lens_projection);
        py_obj->lens_projection = NULL;
    }
}

/*f python_lens_projection_getattr
 */
static PyObject *
python_lens_projection_getattr(PyObject *self, char *attr)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;
    
    if (py_obj->lens_projection) {
        if (!strcmp(attr, "frame_width")) {
            return PyFloat_FromDouble(py_obj->lens_projection->get_frame_width());
        }
        if (!strcmp(attr, "focal_length")) {
            return PyFloat_FromDouble(py_obj->lens_projection->get_focal_length());
        }
        if (!strcmp(attr, "width")) {
            return PyFloat_FromDouble(py_obj->lens_projection->get_sensor_width());
        }
        if (!strcmp(attr, "height")) {
            return PyFloat_FromDouble(py_obj->lens_projection->get_sensor_height());
        }
        if (!strcmp(attr, "orientation")) {
            return python_quaternion_from_c(py_obj->lens_projection->get_orientation().copy());
        }
    }
    return Py_FindMethod(python_lens_projection_methods, self, attr);
}

/*f python_lens_projection_getattr
 */
static int       
python_lens_projection_setattr(PyObject *self, char *attr, PyObject *v)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;
    
    if (py_obj->lens_projection) {
        c_lens_projection *lp = py_obj->lens_projection;
        if (!strcmp(attr, "frame_width")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                lp->set_lens(v_d, lp->get_focal_length(), lp->get_lens_type());
                return 0;
            }
        }
        if (!strcmp(attr, "focal_length")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                lp->set_lens(lp->get_frame_width(), v_d, lp->get_lens_type());
                return 0;
            }
        }
        if (!strcmp(attr, "lens_type")) {
            char *lens_type = PyString_AsString(v);
            if (!lens_type) return -1;
            t_lens_projection_type lp_type = c_lens_projection::lens_projection_type(lens_type);
            lp->set_lens(lp->get_frame_width(), lp->get_focal_length(), lp_type);
            return 0;
        }

        if (!strcmp(attr, "width")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                lp->set_sensor(v_d, lp->get_sensor_height());
                return 0;
            }
        }
        if (!strcmp(attr, "height")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                lp->set_sensor(lp->get_sensor_width(), v_d);
                return 0;
            }
        }
    }
    return -1;
}

/*a Python object
 */
/*f python_lens_projection_init_premodule
 */
int python_lens_projection_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_lens_projection) < 0)
        return -1;
    return 0;
}

/*f python_lens_projection_init_postmodule
 */
void python_lens_projection_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_lens_projection);
    PyModule_AddObject(module, "lens_projection", (PyObject *)&PyTypeObject_lens_projection);
    PyModule_AddObject(module, "lens_projection", (PyObject *)&PyTypeObject_lens_projection);
}

/*f python_lens_projection_new
 */
static PyObject *
python_lens_projection_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj;
    py_obj = (t_PyObject_lens_projection *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->lens_projection = NULL;
    }
    return (PyObject *)py_obj;
}

/*f python_lens_projection_init
 */
static int
python_lens_projection_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;

    double width = 2.0;
    double height = 0.0;
    double focal_length = 35.0;
    double frame_width = 36.0;
    const char *lens_type=NULL;

    static const char *kwlist[] = {"width", "height", "focal_length", "frame_width", "lens_type", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dddds", (char **)kwlist, 
                                     &width, &height, &focal_length, &frame_width, &lens_type))
        return -1;

    py_obj->lens_projection = new c_lens_projection();
    if (!py_obj->lens_projection)
        return -1;

    if (!py_obj->lens_projection->set_polynomial(lens_type)) {
        t_lens_projection_type lp_type = c_lens_projection::lens_projection_type("polynomial");
        py_obj->lens_projection->set_lens(frame_width, focal_length, lp_type);
        py_obj->lens_projection->set_polynomial(lens_type);
    } else {
        t_lens_projection_type lp_type = c_lens_projection::lens_projection_type(lens_type);
        py_obj->lens_projection->set_lens(frame_width, focal_length, lp_type);
    }

    height = (height==0)?width:height;
    py_obj->lens_projection->set_sensor(width, height);

    return 0;
}

/*a External data access
 */
/*f python_lens_projection_data
 */
int
python_lens_projection_data(PyObject* self, int id, void *data_ptr)
{
    t_PyObject_lens_projection *py_obj = (t_PyObject_lens_projection *)self;
    if (!PyObject_TypeCheck(self, &PyTypeObject_lens_projection))
        return 0;

    ((c_lens_projection **)data_ptr)[0] = py_obj->lens_projection;
    return 1;
}


