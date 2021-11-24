#if !defined(_BTKPY_INTERNAL)
#define _BTKPY_INTERNAL

#include <Python.h>
#include <exception>

#include <pybind11/pybind11.h>
#include <Btk/string.hpp>

#define PYBTK_DEF_WIDGET(W) \
    struct Py##W:public Btk::W{ \
        using W::W;\
    };

namespace pybind11 { namespace detail {
    template <> struct type_caster<Btk::u8string> {
    public:
        /**
         * This macro establishes the name 'inty' in
         * function signatures and declares a local variable
         * 'value' of type inty
         */
        PYBIND11_TYPE_CASTER(Btk::u8string, _("Btk::u8string"));

        /**
         * Conversion part 1 (Python->C++): convert a PyObject into a inty
         * instance or return false upon failure. The second argument
         * indicates whether implicit conversions should be applied.
         */
        bool load(handle src, bool) {
            /* Extract PyObject from handle */
            PyObject *source = src.ptr();
            /* Try converting into a Python integer value */
            const char *s = PyUnicode_AsUTF8(source);
            if (!s)
                return false;
            /* Now try to convert into a C++ int */
            value = s;
            /* Ensure return code was OK (to avoid out-of-range errors etc) */
            return (!PyErr_Occurred());
        }

        /**
         * Conversion part 2 (C++ -> Python): convert an inty instance into
         * a Python object. The second and third arguments are used to
         * indicate the return value policy and parent object (for
         * ``return_value_policy::reference_internal``) and are generally
         * ignored by implicit casters.
         */
        static handle cast(Btk::u8string src, return_value_policy /* policy */, handle /* parent */) {
            return PyUnicode_FromString(src.c_str());
        }
    };
    template <> struct type_caster<Btk::u8string_view> {
    public:
        /**
         * This macro establishes the name 'inty' in
         * function signatures and declares a local variable
         * 'value' of type inty
         */
        PYBIND11_TYPE_CASTER(Btk::u8string_view, _("Btk::u8string_view"));

        /**
         * Conversion part 1 (Python->C++): convert a PyObject into a inty
         * instance or return false upon failure. The second argument
         * indicates whether implicit conversions should be applied.
         */
        bool load(handle src, bool) {
            /* Extract PyObject from handle */
            PyObject *source = src.ptr();
            /* Try converting into a Python integer value */
            const char *s = PyUnicode_AsUTF8(source);
            if (!s)
                return false;
            /* Now try to convert into a C++ int */
            value = s;
            /* Ensure return code was OK (to avoid out-of-range errors etc) */
            return (!PyErr_Occurred());
        }

        /**
         * Conversion part 2 (C++ -> Python): convert an inty instance into
         * a Python object. The second and third arguments are used to
         * indicate the return value policy and parent object (for
         * ``return_value_policy::reference_internal``) and are generally
         * ignored by implicit casters.
         */
        static handle cast(Btk::u8string_view src, return_value_policy /* policy */, handle /* parent */) {
            return PyUnicode_FromStringAndSize(src.data(),src.size());
        }
    };
}} // namespace pybind11::detail

namespace PyBtk{
    namespace py= pybind11;
    //Type decl
    //Exceptions
}
#endif // _BTKPY_INTERNAL
