#ifndef OBJECT_V2_OBJECT_H
#define OBJECT_V2_OBJECT_H

#include "object_v2-type.h"

OBJECT_DECLARE (Object, object)
#define OBJECT(any_obj) ((Object*)(any_obj))
struct _Object
{
        o_dest_func _dest_func;
        int         ref;
};

#define OBJECT_DEST_FUNC(any_func) ((o_dest_func)(any_func))

/*为 Object 对象设置销毁函数*/
void _object_set_destory_func (Object* obj, o_dest_func df);
/*增加 Object 节点的引用一次*/
void _object_addref (Object* obj);
/*减少 Object 节点的引用一次*/
void _object_unref (Object* obj);
/*外部引用一次 Object 对象*/
Object* _object_reference (Object* obj);

/*为 Object 对象设置销毁函数*/
#define object_set_destory_func(obj, dest_func) \
        _object_set_destory_func (OBJECT (obj), OBJECT_DEST_FUNC (dest_func))
/*增加 Object 节点的引用一次*/
#define object_addref(any)    _object_addref (OBJECT (any))
/*减少 Object 节点的引用一次*/
#define object_unref(any)     _object_unref (OBJECT (any))
/*外部引用一次 Object 对象*/
#define object_reference(any) _object_reference (OBJECT (any))
/*外部作为 OBJECT_TYPE 类型引用一次 Object 对象*/
#define object_reference_to(any, OBJECT_TYPE) \
        (OBJECT_TYPE (object_reference (any)))

#endif