#include "object_v2-object.h"

void
object_init (Object* obj)
{
        if (obj) {
                obj->ref        = 1;
                obj->_dest_func = NULL;
        }
}

Object*
object_new ()
{
        Object* obj;
        obj = OBJECT_NEW (Object);
        object_init (obj);
        return obj;
}

void
object_destory (Object* obj)
{
        if (obj)
                free (obj);
}

void
_object_set_destory_func (Object* obj, o_dest_func df)
{
        if (obj)
                obj->_dest_func = df;
}

inline void
_object_addref (Object* obj)
{
        if (obj)
                obj->ref += 1;
}

inline void
_object_unref (Object* obj)
{
        if (!obj)
                return;
        obj->ref -= 1;
        if (obj->ref == 0) {
                if (obj->_dest_func)
                        obj->_dest_func (obj);
                else
                        object_destory (obj);
        }
}

Object*
_object_reference (Object* obj)
{
        if (!obj)
                return NULL;
        object_addref (obj);
        return obj;
}