#include "FileObject.h"

#include "FileHash.h"
#include "IOStream.h"

#include <stdio.h>

void
file_object_init (FileObject* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, file_object_destory);
        obj->file_name   = NULL;
        obj->full_path   = NULL;
        obj->revise_time = 0L;
}

FileObject*
file_object_new ()
{
        FileObject* obj;
        obj = OBJECT_NEW (FileObject);
        file_object_init (obj);
        return obj;
}

void
file_object_destruction (FileObject* obj)
{
        if (!obj)
                return;
        object_unref (obj->file_name);
        object_unref (obj->full_path);
}

void
file_object_destory (FileObject* obj)
{
        if (!obj)
                return;
        file_object_destruction (obj);
        free (obj);
}

bool
file_object_set_path (FileObject* obj,
                      const char* full_path,
                      const char* relative_path)
{
        FILE* fp;
        if (!obj || !full_path)
                return false;
        /*检测是否为文件夹或空文件*/
        if (io_stream_is_directory (full_path))
                return false;

        if (!obj->full_path)
                obj->full_path = object_string_new ();
        object_string_set_string (obj->full_path, full_path);

        if (!obj->file_name)
                obj->file_name = object_string_new ();
        object_string_set_string (obj->file_name, relative_path);
        
        return true;
}

int
file_object_time_compare_with (const FileObject* obj, const FileObject* cmp_obj)
{
        if (!obj || !cmp_obj)
                return 0;
        if (obj->revise_time > cmp_obj->revise_time)
                return 1;
        if (obj->revise_time == cmp_obj->revise_time)
                return 0;
        if (obj->revise_time < cmp_obj->revise_time)
                return -1;
}

bool
file_object_hash_compare_with (const FileObject* obj, const FileObject* cmp_obj)
{
        /*如果传参错误，需要保持不修改文件*/
        if (!obj || !cmp_obj)
                return true;
        /*愚蠢的写法*/
        if (obj->file_hash.p1 != cmp_obj->file_hash.p1)
                return false;
        if (obj->file_hash.p2 != cmp_obj->file_hash.p2)
                return false;
        if (obj->file_hash.p3 != cmp_obj->file_hash.p3)
                return false;
        if (obj->file_hash.p4 != cmp_obj->file_hash.p4)
                return false;
        return true;
}

void
file_object_pull (FileObject* obj)
{
        if (!obj || !obj->file_name)
                return;
        obj->revise_time = io_stream_get_revise_time (
                object_string_get_string (obj->full_path));
        file_hash_generate (&obj->file_hash,
                            object_string_get_string (obj->full_path));
}