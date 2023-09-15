#include "IOStream.h"

#include <stdio.h>

#if defined(PROJECTSYNC_SYSTEM_WIN)
        #include <io.h>
        #define io_stream_directory_is_exist(path) \
                (_access (path, 00) == 0 ? true : false)
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        #include <unistd.h>
        #define io_stream_directory_is_exist(path) \
                (access (path, F_OK) == 0 ? true : false)
#endif

bool
io_stream_is_directory (const char* name)
{
#if defined(PROJECTSYNC_SYSTEM_WIN)
        windows_code;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        
#endif
}

size_t
io_stream_get_revise_time (const char* name)
{
        /**
         * 慎重编写：需要兼容不同的平台
         * 需要支持的平台：
         *      GNU/Linux
         *      Windows
         */
}

IOStreamDirectory*
io_stream_directory_open (const char* name, const char* original_path)
{}

void
io_stream_directory_iter (IOStreamDirectory* dir)
{}

void
io_stream_directory_close (IOStreamDirectory* dir)
{}

/**
 * 将文件名称除去，只保留目录名
 */
static void
io_stream_file_catch_dirent_path (ObjectString* obj,
                                  const char*   target_full_path)
{}

/**
 * 创建目录，可能需要连续创建若干目录，创建成功返回 0
 *
 * 谨慎编写，需要兼容：
 * GNU/Linux
 * Windows
 */
static int
io_stream_directory_create (const char* path)
{}

void
io_stream_file_copy (const char* src_full_path, const char* target_full_path)
{
        char*         chunk = NULL;
        FILE*         fp;
        size_t        fsize;
        ObjectString* target_path; /*目标不包含文件名的完整路径*/

        if (!src_full_path || !target_full_path) {
                fprintf (stderr, "io_stream_file_copy (): 传入的路径为空。\n");
                return;
        }

        fp = fopen (src_full_path, "r");
        if (!fp) {
                fprintf (stderr,
                         "io_stream_file_copy (): 错误，无法打开 %s\n",
                         src_full_path);
                return;
        }
        fseek (fp, 0, SEEK_END);
        fsize = ftell (fp);

        /*检查目标的路径是否存在，如果不存在则需要创建对应的目录*/
        target_path = object_string_new ();
        io_stream_file_catch_dirent_path (target_path, target_full_path);
        if (!io_stream_directory_is_exist (target_path->charset) &&
            io_stream_directory_create (target_path->charset)) {
                fprintf (stderr,
                         "io_stream_file_copy (): "
                         "io_stream_directory_create (): 错误: "
                         "创建目录失败！\n");
                object_unref (target_path);
                return;
        }

        /*打开目标文件，开始写入数据*/
        if (fsize) {
                /*文件有大小则执行以下操作*/
                chunk = (char*)malloc (fsize);
                rewind (fp);
                fread (O_PTR (chunk), fsize, 1, fp);
                fclose (fp);
        }
        fp = fopen (target_full_path, "w");
        if (!fp) {
                fprintf (stderr,
                         "io_stream_file_copy (): fopen (): 错误，使用 w "
                         "模式打开文件 '%s' 失败。\n",
                         target_full_path);
                if (chunk)
                        free (chunk);
                object_unref (target_path);
                return;
        }
        rewind (fp);
        if (fsize)
                fwrite (chunk, fsize, 1, fp);
        fclose (fp);
        if (chunk)
                free (chunk);
        object_unref (target_path);
}

void
io_stream_file_delete (const char* target_full_path)
{}
