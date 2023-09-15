#include "QueueObject.h"

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
        if (!obj->target_path)
                obj->target_path = object_string_new ();
        object_string_set_string (obj->target_path, target_path);
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
        QueueObject *tmp = obj_head, *will_dest;
        while (tmp) {
                switch (tmp->action) {
                case QUEUE_ACTION_ADD:
                        break;
                case QUEUE_ACTION_REVISE:
                        break;
                case QUEUE_ACTION_DELETE:
                        break;
                case QUEUE_ACTION_CHECK:
                        break;
                default:
                        break;
                }
                will_dest = tmp;
                tmp       = QUEUE_OBJECT (object_node_get_next (tmp));
                object_unref (will_dest);
        }
}
