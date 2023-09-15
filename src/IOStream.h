#ifndef IO_STREAM_H
#define IO_STREAM_H

#include "../libs/object_v2/object_v2.h"

/*判断是否为文件夹，空文件或文件夹都返回 true*/
bool io_stream_is_directory (const char* name);
/*获取文件的修改时间*/
size_t io_stream_get_revise_time (const char* name);

typedef enum _IOStreamDirectoryType IOStreamDirectoryType;
enum _IOStreamDirectoryType
{
        IO_STREAM_DIRECTORY_TYPE_FILE,
        IO_STREAM_DIRECTORY_TYPE_DIR,
        IO_STREAM_DIRECTORY_TYPE_NONE
};

/**
 * 目录对象：
 * |----------------------------------------------------------------------|
 * |original_path:| /tmp        | /home/user/dirent        | mydir/       |
 * |----------------------------------------------------------------------|
 * |full_path:    | /tmp/a/dir1 | /home/user/dirent/a/dir1 | mydir/a/dir1 |
 * |----------------------------------------------------------------------|
 * |relative_path:| a/dir1      | a/dir1                   | a/dir1       |
 * |----------------------------------------------------------------------|
 */
OBJECT_DECLARE (IOStreamDirectory, io_stream_directory)
#define IO_STREAM_DIRECTORY(any) ((IOStreamDirectory*)(any))
struct _IOStreamDirectory
{
        ObjectString full_path_parent_instance; /*父类代表当前目录的完整路径*/
        ObjectString* relative_path; /*相对操作目录的路径名*/
        char*         iter_name;     /*代表相对于当前目录的文件名*/
        IOStreamDirectoryType type;
        /**
         * 请注意谨慎编写，此处需要兼容不同平台：
         * GNU/Linux
         * Windows
         */
};

/**
 * 打开目录时同时生成 relative_path
 */
IOStreamDirectory* io_stream_directory_open (const char* name,
                                             const char* original_path);
/**
 * 编写时请注意：需要跳过 "." ".." 目录！！！
 */
void io_stream_directory_iter (IOStreamDirectory* dir);
void io_stream_directory_close (IOStreamDirectory* dir);

#endif