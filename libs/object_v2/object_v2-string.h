#ifndef OBJECT_V2_STRING_H
#define OBJECT_V2_STRING_H

#include "object_v2-node.h"
#include "object_v2-object.h"

OBJECT_DECLARE (ObjectString, object_string)
#define OBJECT_STRING(any) ((ObjectString*)(any))
struct _ObjectString
{
        Object parent;
        int    length;
        char*  charset;
        char*  cache;
};

ObjectString* object_string_new_with (const char* str);
/*设置字符串对象的内容*/
void        object_string_set_string (ObjectString* obj, const char* str);
/*获取字符串对象的内容*/
const char* object_string_get_string (ObjectString* obj);
/*向字符串末尾添加新的字符串*/
void object_string_append_string (ObjectString* obj, const char* add_str);
/*打印字符串对象*/
void object_string_println (ObjectString* obj);
/*从标准输入获取字符串对象*/
void object_string_getln (ObjectString* obj);

/*比较字符串对象内容是否相等*/
bool object_string_compare (const ObjectString* obja, const ObjectString* objb);
/*比较字符串对象与另一个字符串内容是否相等*/
bool object_string_compare_with_charset (const ObjectString* obj,
                                         const char*         charset);
/*将字符串对象的内容复制到一枚指针中，谨慎使用*/
void object_string_copy_to_charset (ObjectString* obj, char** charset);

/*引用一个字符串（引用计数增加一）*/
ObjectString* object_string_reference (ObjectString* obj);

OBJECT_DECLARE (ObjectStrings, object_strings)
#define OBJECT_STRINGS(any) ((ObjectStrings*)(any))
struct _ObjectStrings
{
        ObjectNode    parent_instance;
        ObjectString* string;
};
/*向字符串链对象添加一个字符串*/
void        object_strings_set_string (ObjectStrings* strings_manager,
                                       const char*    set_charset);
/*删除字符串链对象中的一个字符串*/
void        object_strings_dest_string (ObjectStrings*  strings_manager,
                                        const char*     dest_charset,
                                        ObjectStrings** manager_var);
/*对字符串链对象进行迭代时，首先设置起始位置*/
void        object_strings_iter_set (ObjectStrings* strings);
/*重置字符串链对象迭代位置*/
void        object_strings_iter_reset (ObjectStrings* strings);
/*迭代一个字符串链对象*/
const char* object_strings_iter_get (ObjectStrings* strings);

#endif