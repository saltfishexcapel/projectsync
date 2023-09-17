#include "ConfigObject.h"

#include "IOStream.h"

#include <stdio.h>

void
config_object_init (ConfigObject* obj)
{
        if (!obj)
                return;
        object_hash_init (OBJECT_HASH (obj));
        object_set_destory_func (obj, config_object_destory);
        obj->path      = NULL;
        obj->is_target = false;
}

ConfigObject*
config_object_new ()
{
        ConfigObject* obj;
        obj = OBJECT_NEW (ConfigObject);
        config_object_init (obj);
        return obj;
}

void
config_object_destruction (ConfigObject* obj)
{
        if (!obj)
                return;
        object_hash_destruction (OBJECT_HASH (obj));
        object_unref (obj->path);
}

void
config_object_destory (ConfigObject* obj)
{
        if (!obj)
                return;
        config_object_destruction (obj);
        free (obj);
}

bool
config_object_set_path (ConfigObject* obj, const char* path, bool is_target)
{
        bool have_path;
        if (!obj || !path)
                return false;

        have_path = io_stream_directory_is_exist (path);
        if (!have_path && !is_target) {
                fprintf (stderr,
                         "config_object_set_path (): "
                         "源路径必须存在！\n");
                return false;
        }
        if (is_target && !have_path && io_stream_directory_create (path)) {
                fprintf (stderr,
                         "config_object_set_path (): 目标路径创建失败。\n");
                return false;
        }
        if (!io_stream_is_directory (path)) {
                fprintf (stderr,
                         "config_object_set_path (): '%s' 必须是目录。\n",
                         path);
                return false;
        }
        if (!obj->path)
                obj->path = object_string_new ();
        object_string_set_string (obj->path, path);
        return true;
}

static const char* global_original_path = NULL;
static o_uint      global_load_nums     = 0;

static void
config_object_pull_dirents (ObjectHash* hash, const char* path_name)
{
        IOStreamDirectory* dir;
        ObjectString *     full_path, *relative_path;
        FileObject*        fobj;

        if (!global_original_path)
                global_original_path = path_name;

        dir = io_stream_directory_open (path_name, global_original_path);
        if (!dir)
                return;
        full_path     = object_string_new ();
        relative_path = object_string_new ();
retry:
        /*将完整路径生成至 full_path*/
        io_stream_directory_iter (dir);
        object_string_set_string (full_path, OBJECT_STRING (dir)->charset);
        object_string_append_string (full_path, DCL);
        object_string_append_string (full_path, dir->iter_name);
        /*将相对路径生成至 relative_path*/
        object_string_set_string (relative_path, dir->relative_path->charset);
        object_string_append_string (relative_path, DCL);
        object_string_append_string (relative_path, dir->iter_name);
        switch (dir->type) {
        case IO_STREAM_DIRECTORY_TYPE_DIR:
                config_object_pull_dirents (hash, full_path->charset);
                break;
        case IO_STREAM_DIRECTORY_TYPE_FILE: {
                printf ("\r载入文件数: %d", (++global_load_nums));
                fflush (stdout);
                fobj = file_object_new ();
                if (!file_object_set_path (fobj,
                                           full_path->charset,
                                           relative_path->charset)) {
                        fprintf (
                                stderr,
                                "config_object_pull_dirents(): "
                                "file_object_set_path():"
                                "\n\t设置路径错误：FileObject::set_path (%s)\n",
                                full_path->charset);
                        goto retry;
                }
                file_object_pull (fobj);
                object_hash_set_value (
                        hash,
                        object_string_get_string (fobj->file_name),
                        false,
                        O_PTR (fobj),
                        OBJECT_DEST_FUNC (_object_unref));
                break;
        }
        default:
                break;
        }

        if (dir->type != IO_STREAM_DIRECTORY_TYPE_NONE)
                goto retry;

        object_unref (full_path);
        object_unref (relative_path);
        io_stream_directory_close (dir);
}

void
config_object_pull (ConfigObject* obj)
{
        if (!obj || !obj->path)
                return;
        printf ("\n正在载入文件信息 '%s' ...\n", obj->path->charset);
        global_load_nums     = 0;
        global_original_path = object_string_get_string (obj->path);
        config_object_pull_dirents (OBJECT_HASH (obj), obj->path->charset);
}

QueueObject*
config_object_compare_with (ConfigObject* obj,
                            ConfigObject* cmp_obj,
                            bool          disable_delete)
{
        QueueObject * queue = NULL, *queue_head = NULL;
        ObjectHashKV* kv;
        FileObject *  fobj, *dist_obj;
        bool          queue_add_flag = true;

        printf ("\n正在比较文件 ...\n");
        /*初始设置哈希表迭代器*/
        object_hash_iter_reset (OBJECT_HASH (obj));
reiter:
        /*迭代表中的每个元素*/
        kv   = object_hash_iter_get (OBJECT_HASH (obj));
        fobj = FILE_OBJECT (kv ? kv->value : NULL);
        /*对源目录迭代完成，下一步迭代操作目录*/
        if (!fobj)
                goto iter_target_path;
        /*寻找目标表中是否有相同的 FileObject*/
        dist_obj = object_hash_get_value (
                OBJECT_HASH (cmp_obj),
                object_string_get_string (fobj->file_name),
                false);
        /*在队列尾新建一个 QueueObject 节点*/
        if (!queue && queue_add_flag) {
                queue_head = queue = queue_object_new ();
                object_node_set_as_manager (OBJECT_NODE (queue_head));
        } else if (queue_add_flag) {
                queue = queue_object_new ();
                object_node_set_new_end (OBJECT_NODE (queue_head),
                                         OBJECT_NODE (queue));
        }
        /*目标目录中没有这个文件对象，需要新增*/
        if (!dist_obj) {
                queue_object_set_action (queue, QUEUE_ACTION_ADD);
                queue_object_set_target_path (
                        queue,
                        object_string_get_string (cmp_obj->path));
                queue_object_set_object (queue, fobj);
                /*设置完成，继续迭代*/
                queue_add_flag = true;
                goto reiter;
        }
        /*源文件比目标文件还旧，且哈希值不同，需要检查*/
        if ((file_object_time_compare_with (fobj, dist_obj) < 0) &&
            (file_object_hash_compare_with (fobj, dist_obj) == false)) {
                queue_object_set_action (queue, QUEUE_ACTION_CHECK);
                queue_object_set_target_path (
                        queue,
                        object_string_get_string (cmp_obj->path));
                queue_object_set_object (queue, fobj);
                queue_add_flag = true;
                goto reiter;
        }
        /*源文件比目标文件新，且哈希值不一样，需要更新*/
        if ((file_object_time_compare_with (fobj, dist_obj) > 0) &&
            (file_object_hash_compare_with (fobj, dist_obj) == false)) {
                queue_object_set_action (queue, QUEUE_ACTION_REVISE);
                queue_object_set_target_path (
                        queue,
                        object_string_get_string (cmp_obj->path));
                queue_object_set_object (queue, fobj);
                queue_add_flag = true;
                goto reiter;
        }

        queue_add_flag = false;
        goto reiter;

        if (disable_delete)
                return queue_head;

iter_target_path:
        /*迭代操作对象目录，检查是否有需要删除的文件*/
        object_hash_iter_reset (OBJECT_HASH (cmp_obj));
reiter_target:
        kv   = object_hash_iter_get (OBJECT_HASH (cmp_obj));
        fobj = FILE_OBJECT (kv ? kv->value : NULL);
        /*迭代完成，返回操作队列*/
        if (!fobj)
                return queue_head;
        /*寻找目标表中是否有相同的 FileObject*/
        dist_obj = object_hash_get_value (
                OBJECT_HASH (obj),
                object_string_get_string (fobj->file_name),
                false);
        /*没找到就代表这是需要删除的文件对象*/
        if (!dist_obj) {
                /*在队列尾新建一个 QueueObject 节点*/
                if (!queue && queue_add_flag) {
                        queue_head = queue = queue_object_new ();
                        object_node_set_as_manager (OBJECT_NODE (queue_head));
                } else if (queue_add_flag) {
                        queue = queue_object_new ();
                        object_node_set_new_end (OBJECT_NODE (queue_head),
                                                 OBJECT_NODE (queue));
                }
                queue_object_set_action (queue, QUEUE_ACTION_DELETE);
                queue_object_set_target_path (
                        queue,
                        object_string_get_string (cmp_obj->path));
                queue_object_set_object (queue, fobj);

                queue_add_flag = true;
        }
        goto reiter_target;
}