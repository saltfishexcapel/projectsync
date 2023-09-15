#ifndef QUEUE_OBJECT_H
#define QUEUE_OBJECT_H

#include "../libs/object_v2/object_v2.h"
#include "FileObject.h"

typedef enum _QueueAction QueueAction;
enum _QueueAction
{
        QUEUE_ACTION_ADD,
        QUEUE_ACTION_REVISE,
        QUEUE_ACTION_DELETE,
        QUEUE_ACTION_CHECK,
        QUEUE_ACTION_NONE
};

OBJECT_DECLARE (QueueObject, queue_object)
#define QUEUE_OBJECT(any) ((QueueObject*)(any))
struct _QueueObject
{
        ObjectNode parent_instance;
        FileObject*
                operate_object; /*将要操作的文件，包含文件相对于操作目录的路径*/
        ObjectString* target_original_path; /*操作的目标目录*/
        QueueAction   action;
};

void queue_object_set_object (QueueObject* obj, FileObject* file_obj);
void queue_object_set_target_path (QueueObject* obj, const char* target_path);
void queue_object_set_action (QueueObject* obj, QueueAction action);
void queue_object_run_action (QueueObject* obj_head);

#endif