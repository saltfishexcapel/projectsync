#ifndef CONFIG_OBJECT_H
#define CONFIG_OBJECT_H

#include "QueueObject.h"
#include "../libs/object_v2/object_v2.h"

/**
 * 目录分隔符
 * 需要做兼容性处理！
 * Windows
 * GNU/Linux
 */
#define DCL "/"

OBJECT_DECLARE (ConfigObject, config_object)
#define CONFIG_OBJECT(any) ((ConfigObject*)(any))
struct _ConfigObject
{
        ObjectHash    parent_instance;
        ObjectString* path;
};

/*设置工作目录，如果目录不存在或不是目录则返回 false*/
bool         config_object_set_path (ConfigObject* obj, const char* path);
void         config_object_pull (ConfigObject* obj);
/*返回一个队列对象，在最后需要解引用这个对象*/
QueueObject* config_object_compare_with (ConfigObject* obj,
                                         ConfigObject* cmp_obj);

#endif