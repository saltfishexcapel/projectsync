#ifndef FILE_OBJECT_H
#define FILE_OBJECT_H

#include "../libs/object_v2/object_v2.h"
#include "FileHash.h"

OBJECT_DECLARE (FileObject, file_object)
#define FILE_OBJECT(any) ((FileObject*)(any))
struct _FileObject
{
        Object        parent_instance;
        ObjectString* file_name; /*文件相对于操作目录的路径*/
        ObjectString* full_path; /*文件的完整相对路径*/
        size_t        revise_time;
        FileHash      file_hash;
};

bool file_object_set_path (FileObject* obj,
                           const char* full_path,
                           const char* relative_path);
/**
 * 如果 obj 的时间比 cmp_obj 新，则返回值为 1，反之为 -1，相同为 0
 */
int file_object_time_compare_with (const FileObject* obj,
                                   const FileObject* cmp_obj);
/**
 * 如果 obj 和 cmp_obj 的哈希相同则返回 true，不相同则返回 false
 */
bool file_object_hash_compare_with (const FileObject* obj,
                                    const FileObject* cmp_obj);
void file_object_pull (FileObject* obj);

#endif