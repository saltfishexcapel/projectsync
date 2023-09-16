#include "IOStream.h"

#include <stdio.h>
#include <string.h>

bool
io_stream_is_directory (const char* name)
{
#if defined(PROJECTSYNC_SYSTEM_WIN)
        DWORD status;
        status = GetFileAttributes (name);
        if (status == INVALID_FILE_ATTRIBUTES)
                return false;
        if (status | FILE_ATTRIBUTE_DIRECTORY)
                return true;
        return false;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        struct stat st;
        if (stat (name, &st))
                return false;
        if (S_ISDIR (st.st_mode))
                return true;
        return false;
#endif
}

size_t
io_stream_get_revise_time (const char* name)
{
#if defined(PROJECTSYNC_SYSTEM_WIN)
        size_t   filetm = 0;
        FILETIME fTime;
        HANDLE   hFile = CreateFile (_T (name),
                                   GENERIC_WRITE | GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   TRUNCATE_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL);
        if (hFile == INVALID_HANDLE_VALUE)
                return 0L;
        GetFileTime (hFile, NULL, NULL, &fTime);
        filetm = fTime.dwHighDateTime;
        filetm <<= sizeof (DWORD);
        filetm |= fTime.dwLowDateTime;
        return filetm;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        struct stat st;
        if (stat (name, &st))
                return 0L;
        return st.st_mtime;
#endif
}

static void
io_stream_directory_catch_relative_path (ObjectString* relative_path,
                                         const char*   full_path,
                                         const char*   original_path)
{
        int len;
        if (strlen (full_path) < strlen (original_path)) {
                fprintf (stderr,
                         "io_stream_directory_catch_relative_path (): "
                         "注意，完整路径无法匹配原路径，相对路径已被设置为 "
                         "'.'\n");
                object_string_set_string (relative_path, ".");
                return;
        }

        len = strlen (original_path);
        while (full_path[len] != '\0' && full_path[len] == _DCL_)
                ++len;
        object_string_set_string (relative_path, &full_path[len]);
        if (relative_path->length == 0)
                object_string_append_string (relative_path, ".");
}

IOStreamDirectory*
io_stream_directory_open (const char* full_path, const char* original_path)
{
        IOStreamDirectory* dobj;

        if (!full_path || !original_path)
                return NULL;
        dobj = OBJECT_NEW (IOStreamDirectory);
        object_string_init (OBJECT_STRING (dobj));
        dobj->relative_path = NULL;
        dobj->dir           = NULL;
        dobj->iter_name     = NULL;
        dobj->type          = IO_STREAM_DIRECTORY_TYPE_NONE;

#if defined(PROJECTSYNC_SYSTEM_WIN)
        windows_code;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        dobj->dir = opendir (full_path);
        if (!dobj->dir) {
                perror ("opendir:");
                fprintf (stderr,
                         "io_stream_directory_open (): 打开 %s 时失败。\n",
                         full_path);
                object_unref (dobj);
                return NULL;
        }
#endif

        object_string_set_string (OBJECT_STRING (dobj), full_path);
        dobj->relative_path = object_string_new ();
        io_stream_directory_catch_relative_path (dobj->relative_path,
                                                 full_path,
                                                 original_path);
        return dobj;
}

void
io_stream_directory_iter (IOStreamDirectory* dir)
{
        ObjectString* full_file_path;
        if (!dir)
                return;

#if defined(PROJECTSYNC_SYSTEM_WIN)
        windows_code;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        struct dirent* dinfo;
retry:
        dinfo = readdir (dir->dir);
        if (!dinfo) {
                dir->type = IO_STREAM_DIRECTORY_TYPE_NONE;
                return;
        }
        if (strcmp (dinfo->d_name, ".") == 0 ||
            strcmp (dinfo->d_name, "..") == 0)
                goto retry;
        dir->iter_name = dinfo->d_name;
        full_file_path = object_string_new ();
        object_string_set_string (full_file_path, OBJECT_STRING (dir)->charset);
        object_string_append_string (full_file_path, DCL);
        object_string_append_string (full_file_path, dinfo->d_name);
        if (io_stream_is_directory (full_file_path->charset))
                dir->type = IO_STREAM_DIRECTORY_TYPE_DIR;
        else
                dir->type = IO_STREAM_DIRECTORY_TYPE_FILE;
        object_unref (full_file_path);
#endif
}

void
io_stream_directory_close (IOStreamDirectory* dir)
{
        if (!dir)
                return;

#if defined(PROJECTSYNC_SYSTEM_WIN)
        windows_code;
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        if (dir->dir)
                closedir (dir->dir);
#endif

        object_unref (dir->relative_path);
        object_string_destruction (OBJECT_STRING (dir));
        free (dir);
}

/**
 * 将文件名称除去，只保留目录名
 */
static void
io_stream_file_catch_dirent_path (ObjectString* obj,
                                  const char*   target_full_path)
{
        char        buf[2]  = {0, 0};
        int         dcl_num = 0;
        const char* p       = target_full_path;
        if (!obj || !target_full_path)
                return;
        /*先获取文件分隔符的数量*/
        if (*p == _DCL_)
                ++p; /*防止开头为分隔符的情况*/
        while (*p != '\0') {
                /* *(p + 1) != '\0' 防止结尾是分隔符的情况*/
                if (*p == _DCL_ && (*(p + 1) != _DCL_ && *(p + 1) != '\0'))
                        ++dcl_num;
                ++p;
        }
        /*没有一个分隔符的话就以 . 作为底层路径*/
        if (dcl_num == 0) {
                object_string_set_string (obj, ".");
                return;
        }

        /*逐个读入字符*/
        p = target_full_path;
        object_string_set_string (obj, "");
        for (int i = 0; i < dcl_num && *p != '\0'; ++p) {
                if (*p != _DCL_ && *(p + 1) == _DCL_)
                        ++i;
                buf[0] = *p;
                object_string_append_string (obj, buf);
        }
}

/**
 * 创建目录，可能需要连续创建若干目录，创建成功返回 0
 *
 * 谨慎编写，需要兼容：
 * GNU/Linux
 * Windows
 */
int
io_stream_directory_create (const char* path)
{
        int status = 1;

#if defined(PROJECTSYNC_SYSTEM_WIN)
        ObjectString* command;
        command = object_string_new ();
        object_string_set_string (command, "md ");
        object_string_append_string (command, path);
        /*系统执行 CMD 指令创建文件夹*/
        status = system (command->charset);
        object_unref (up_path);
#elif defined(PROJECTSYNC_SYSTEM_LINUX)
        ObjectString* up_path;
        /*检测当前目录是否存在*/
        if (io_stream_directory_is_exist (path))
                return 0;
        /*当前目录不存在，递归至上层目录*/
        up_path = object_string_new ();
        io_stream_file_catch_dirent_path (up_path, path);
        status = io_stream_directory_create (up_path->charset);
        if (status) {
                fprintf (stderr,
                         "io_stream_directory_create (): 创建 %s "
                         "出错。\n\t%s 不会被创建\n",
                         up_path->charset,
                         path);
                object_unref (up_path);
                return status;
        }
        status = mkdir (path, 0755);
        if (status)
                perror ("mkdir ():");
        object_unref (up_path);
#endif

        return status;
}

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
                         "io_stream_file_copy (): fopen (): 错误，使用 "
                         "w "
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
{
        /*使用 GLIBC 的删除方法*/
        if (remove (target_full_path)) {
                perror ("remove:");
                fprintf (stderr,
                         "io_stream_file_delete (): 移除文件发生错误: "
                         "%s\n",
                         target_full_path);
        }
}
