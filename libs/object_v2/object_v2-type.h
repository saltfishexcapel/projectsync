#ifndef OBJECT_V2_TYPE_H
#define OBJECT_V2_TYPE_H

#include <stdbool.h>
#include <stdlib.h>

typedef void*         o_ptr;
typedef long          o_long;
typedef unsigned long o_ulong;
typedef long          o_type8;
typedef unsigned int  o_uint;
typedef void (*o_dest_func) (void*);

#define INT(any)    (int)(any)
#define O_UINT(any) (o_uint) (any)
#define O_PTR(any)  ((o_ptr)(any))

/*声明一个对象*/
#define OBJECT_DECLARE(ObjName, obj_name)                                \
        typedef struct _##ObjName ObjName;                               \
        void                      obj_name##_init (ObjName* obj);        \
        ObjName*                  obj_name##_new ();                     \
        void                      obj_name##_destruction (ObjName* obj); \
        void                      obj_name##_destory (ObjName* obj);

#define OBJECT_NEW(ObjectType) ((ObjectType*)malloc (sizeof (ObjectType)))

#endif