#ifndef CONFIG_OBJECT_H
#define CONFIG_OBJECT_H

#include "../libs/object_v2/object_v2.h"
#include "QueueObject.h"
#include "config.h"

OBJECT_DECLARE (ConfigObject, config_object)
#define CONFIG_OBJECT(any) ((ConfigObject*)(any))
struct _ConfigObject
{
        ObjectHash    parent_instance;
        ObjectString* path;
        bool          is_target;
};

/*设置工作目录，如果目录不存在或不是目录则返回 false*/
bool
config_object_set_path (ConfigObject* obj, const char* path, bool is_target);
void config_object_pull (ConfigObject* obj);
/*返回一个队列对象，在最后需要解引用这个对象*/
QueueObject* config_object_compare_with (ConfigObject* obj,
                                         ConfigObject* cmp_obj,
                                         bool disable_delete);

#endif