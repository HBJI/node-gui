#ifndef IMPL_VALUE_GTK_H
#define IMPL_VALUE_GTK_H

#include <iosfwd>

#include <glib-object.h>
#include <node.h>

#include "node_gui_object.h"

namespace clip {
// Note, I don't provide destructor and copy constructor for MovedGValue,
// so when MovedGValue is copied, we can get the same effect of 'move'.
// My main purpose is to move GValue between threads, which is not
// provided by lambda functions.
class MovedGValue {
public:
    MovedGValue (v8::Handle<Value> value)
    {
        handle_ = { 0 };

        if (value->IsUndefined ()) {
            g_value_init (&handle_, G_TYPE_NONE);
        } else if (value->IsNull ()) {
            g_value_init (&handle_, G_TYPE_NONE);
        } else if (value->IsString ()) {
            g_value_init (&handle_, G_TYPE_STRING);
            g_value_set_string (&handle_, *String::Utf8Value (value));
        } else if (value->IsBoolean ()) {
            g_value_init (&handle_, G_TYPE_BOOLEAN);
            g_value_set_boolean (&handle_, value->BooleanValue ());
        } else if (value->IsNumber ()) {
            g_value_init (&handle_, G_TYPE_DOUBLE);
            g_value_set_boolean (&handle_, value->NumberValue ());
        } else if (value->IsExternal ()) {
            // External holds glib object
            g_value_init (&handle_, G_TYPE_OBJECT);
            g_value_set_object (&handle_, v8::External::Unwrap (value));
        } else if (value->IsInt32 ()) {
            g_value_init (&handle_, G_TYPE_INT);
            g_value_set_int (&handle_, value->Int32Value ());
        } else if (value->IsUint32 ()) {
            g_value_init (&handle_, G_TYPE_UINT);
            g_value_set_uint (&handle_, value->Uint32Value ());
        } else {
            g_value_init (&handle_, G_TYPE_INVALID);
        }
    }

    operator GValue* () {
        return &handle_;
    }

    operator const GValue* () const {
        return &handle_;
    }

private:
    GValue handle_;
};

// Generic GValue to v8::Value
v8::Handle<Value> v8value (GValue* value) {
    HandleScope scope;

    if (value == NULL)
        return scope.Close (Undefined ());

    switch (G_TYPE_FUNDAMENTAL(G_VALUE_TYPE(value))) {
        case G_TYPE_INVALID:
        case G_TYPE_NONE:
            return scope.Close (Undefined ());

        case G_TYPE_BOOLEAN:
            return scope.Close (Boolean::New (g_value_get_boolean (value)));

        case G_TYPE_INT:
            return scope.Close (Integer::New (g_value_get_int (value)));

        case G_TYPE_LONG:
            return scope.Close (Number::New (g_value_get_long (value)));

        case G_TYPE_UINT:
            return scope.Close (Integer::NewFromUnsigned (g_value_get_uint (value)));

        case G_TYPE_ULONG:
            return scope.Close (Integer::NewFromUnsigned (g_value_get_ulong (value)));

        case G_TYPE_FLOAT:
            return scope.Close (Number::New (g_value_get_float (value)));

        case G_TYPE_ENUM:
            return scope.Close (Integer::New (g_value_get_enum (value)));

        case G_TYPE_FLAGS:
            return scope.Close (Integer::New (g_value_get_flags (value)));

        case G_TYPE_STRING:
            return scope.Close (String::New (g_value_get_string (value)));

        case G_TYPE_OBJECT:
            return scope.Close (Object::NewInstance (g_value_get_object (value)));

        default:
            return ThrowException(Exception::TypeError(
                        String::New("Cannot find equivanent type")));
    }
}
} /* clip */

#endif /* end of IMPL_VALUE_GTK_H */