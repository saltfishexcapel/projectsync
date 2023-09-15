#include "QueueObject.h"

#include "IOStream.h"

#include <stdio.h>

void
queue_object_init (QueueObject* obj)
{
        if (!obj)
                return;
        object_node_init (OBJECT_NODE (obj));
        object_set_destory_func (obj, queue_object_destory);
        obj->action         = QUEUE_ACTION_NONE;
        obj->operate_object = NULL;
}

QueueObject*
queue_object_new ()
{
        QueueObject* obj;
        obj = OBJECT_NEW (QueueObject);
        queue_object_init (obj);
        return obj;
}

void
queue_object_destruction (QueueObject* obj)
{
        if (!obj)
                return;
        /**
         * QueueObject 单独解引用，不会解除下个节点的引用
         */
        if (object_node_get_next (OBJECT_NODE (obj)))
                object_addref (object_node_get_next (OBJECT_NODE (obj)));
        object_node_destruction (OBJECT_NODE (obj));
        object_unref (obj->operate_object);
}

void
queue_object_destory (QueueObject* obj)
{
        if (!obj)
                return;
        queue_object_destruction (obj);
        free (obj);
}

void
queue_object_set_object (QueueObject* obj, FileObject* file_obj)
{
        if (!obj || !file_obj)
                return;
        obj->operate_object = object_reference_to (file_obj, FILE_OBJECT);
}

void
queue_object_set_target_path (QueueObject* obj, const char* target_path)
{
        if (!obj || !target_path)
                return;
        if (!obj->target_original_path)
                obj->target_original_path = object_string_new ();
        object_string_set_string (obj->target_original_path, target_path);
}

void
queue_object_set_action (QueueObject* obj, QueueAction action)
{
        if (!obj || (action < QUEUE_ACTION_ADD || action > QUEUE_ACTION_NONE))
                return;
        obj->action = action;
}

void
queue_object_run_action (QueueObject* obj_head)
{
        QueueObject * tmp = obj_head, *will_dest;
        ObjectString* target_full_path;

        if (!obj_head)
                return;
        target_full_path = object_string_new ();

        while (tmp) {
                /*将操作路径设置为 original_path/relative_path*/
                object_string_set_string (target_full_path,
                                          tmp->target_original_path->charset);
                object_string_append_string (target_full_path, "/");
                object_string_append_string (
                        target_full_path,
                        tmp->operate_object->file_name->charset);
                switch (tmp->action) {
                case QUEUE_ACTION_REVISE:
                case QUEUE_ACTION_ADD:
                        io_stream_file_copy (
                                tmp->operate_object->full_path->charset,
                                target_full_path->charset);
                        break;
                case QUEUE_ACTION_DELETE:
                        io_stream_file_delete (
                                tmp->operate_object->full_path->charset);
                        break;
                case QUEUE_ACTION_CHECK:
                        printf ("queue_object_run_action(): "
                                "请注意，目标文件比源文件新！请手动检查需要同步"
                                "哪个文件。\n目标文件: %s\n源文件: %s\n",
                                target_full_path->charset,
                                tmp->operate_object->full_path->charset);
                        break;
                default:
                        break;
                }
                will_dest = tmp;
                tmp       = QUEUE_OBJECT (object_node_get_next (tmp));
                object_unref (will_dest);
        }
        object_unref (target_full_path);
}
