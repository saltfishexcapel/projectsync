#ifndef IO_STREAM_H
#define IO_STREAM_H

#include "../libs/object_v2/object_v2.h"
#include "config.h"

/**
 * 目录分隔符
 * 需要做兼容性处理！
 * Windows
 * GNU/Linux
 */
#if defined(PROJECTSYNC_SYSTEM_WIN)
        #define DCL   "\\"
        #define _DCL_ '\\'
        #include <fileapi.h>
        #include <io.h>
        #include <stdlib.h>
        #include <windows.h>
        #define io_stream_directory_is_exist(path) \
                (_access (path, 00) == 0 ? true : false)
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        #define DCL   "/"
        #define _DCL_ '/'
        #include <dirent.h>
        #include <sys/stat.h>
        #include <unistd.h>
        #define io_stream_directory_is_exist(path) \
                (access (path, F_OK) == 0 ? true : false)
#endif

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
#if defined(PROJECTSYNC_SYSTEM_WIN)
        windows_code;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        DIR* dir;
#endif
};

/*void io_stream_file_catch_dirent_path (ObjectString* obj,
                                       const char*   target_full_path);*/
/**
 * 打开目录时同时生成 relative_path
 */
IOStreamDirectory* io_stream_directory_open (const char* full_path,
                                             const char* original_path);
/**
 * 编写时请注意：需要跳过 "." ".." 目录！！！
 */
void io_stream_directory_iter (IOStreamDirectory* dir);
/**
 * 退出的时候要解引用对象
 */
void io_stream_directory_close (IOStreamDirectory* dir);

/**
 * 创建目录，可能需要连续创建若干目录，创建成功返回 0
 *
 * 谨慎编写，需要兼容：
 * GNU/Linux
 * Windows
 */
int io_stream_directory_create (const char* path);

/**
 * 文件拷贝，需要完成的操作有：
 * 一、检查目标文件所处的目录是否被创建
 * 二、文件的完整拷贝
 *
 * 谨慎编写，需要兼容的平台有：
 * GNU/Linux
 * Windows
 */
void io_stream_file_copy (const char* src_full_path,
                          const char* target_full_path);
/**
 * 文件删除
 *
 * 谨慎编写，需要兼容的平台有：
 * GNU/Linux
 * Windows
 */
void io_stream_file_delete (const char* target_full_path);

#endif