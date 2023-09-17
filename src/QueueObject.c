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
        obj->action               = QUEUE_ACTION_NONE;
        obj->operate_object       = NULL;
        obj->target_original_path = NULL;
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

        object_node_destruction (OBJECT_NODE (obj));
        object_unref (obj->operate_object);
        object_unref (obj->target_original_path);
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
queue_object_run_action (QueueObject* obj_head, bool enable_verbose)
{
        o_uint operate_num = 0, add = 0, revise = 0, delete = 0, check = 0;
        QueueObject * tmp = obj_head, *will_dest = NULL;
        ObjectString* target_full_path;

        if (!obj_head)
                return;
        printf ("开始操作文件队列 ...\n");
        target_full_path = object_string_new ();
        while (tmp) {
                /*将操作路径设置为 original_path/relative_path*/
                if (tmp->action != QUEUE_ACTION_NONE) {
                        object_string_set_string (
                                target_full_path,
                                tmp->target_original_path->charset);
                        object_string_append_string (target_full_path, "/");
                        object_string_append_string (
                                target_full_path,
                                tmp->operate_object->file_name->charset);
                        /*操作数加一*/
                        operate_num += 1;
                }
                switch (tmp->action) {
                case QUEUE_ACTION_REVISE: {
                        io_stream_file_copy (
                                tmp->operate_object->full_path->charset,
                                target_full_path->charset);
                        if (enable_verbose) {
                                printf ("\033[01;32m更新\033[0m '%s'\n",
                                        target_full_path->charset);
                        }
                        revise += 1;
                        break;
                }
                case QUEUE_ACTION_ADD: {
                        io_stream_file_copy (
                                tmp->operate_object->full_path->charset,
                                target_full_path->charset);
                        if (enable_verbose) {
                                printf ("\033[01;32m添加\033[0m '%s'\n",
                                        target_full_path->charset);
                        }
                        add += 1;
                        break;
                }
                case QUEUE_ACTION_DELETE: {
                        io_stream_file_delete (
                                tmp->operate_object->full_path->charset);
                        if (enable_verbose) {
                                printf ("\033[01;31m删除\033[0m '%s'\n",
                                        tmp->operate_object->full_path
                                                ->charset);
                        }
                        delete += 1;
                        break;
                }
                case QUEUE_ACTION_CHECK:
                        printf ("\033[01;33m检查\033[0m "
                                "请注意，目标文件 '%s' 比源文件 '%s' "
                                "更新！请手动检查需要同步"
                                "哪个文件。\n",
                                target_full_path->charset,
                                tmp->operate_object->full_path->charset);
                        check += 1;
                        break;
                default:
                        break;
                }
                will_dest = tmp;
                tmp       = object_reference_to (
                        object_node_get_next (OBJECT_NODE (tmp)),
                        QUEUE_OBJECT);
                object_unref (will_dest);
        }

        if (!operate_num) {
                printf ("无需任何操作。\n");
        } else {
                printf ("成功操作了 %u 个文件。\n", operate_num);
                printf ("新增(%u), 更新(%u), 删除(%u), 需要手动处理(%u)\n",
                        add,
                        revise,
                        delete,
                        check);
        }
        object_unref (target_full_path);
}
