#include "../libs/object_v2/args_parser.h"
#include "ConfigObject.h"

#include <stdio.h>

const char* global_help[] = {
        "projsync v0.1 by 咸鱼雨涵/SaltFishExcapel\n",
        "\n用法: projsync [option] [[src] [target]]\n"
        "\tsrc:\t需要同步的目录（必须存在）\n"
        "\ttarget:\t目标目录（若不存在则将被创建）\n\n"
        "\toption:\t选项\n"
        "\t-d/--disable-delete\t启用后，将不会删除文件。\n"
        "\t-v/--verbose\t\t启用后，将输出所有操作的文件详情。\n"
        "\t--usage\t\t\t显示一些使用案例。\n"
        "\t--help\t\t\t显示本帮助。\n"
        "\t--version\t\t程序版本。\n",
        "\n使用举例:\n"
        "projsync myprojectdir1 backupdir\n"
        "projsync /home/user/datas/ /home/user/backup/\n"
        "projsync --verbose dir1 dir2\n"
        "projsync --disable-delete --verbose dir1 dir2\n"
        "projsync -d -v dir1 dir2\n"
        "projsync -dv dir1 dir2\n"
        "projsync --help\n"};

int
main (int argc, const char** argv)
{
        const Args*   args;
        ArgsObject*   argobj;
        ConfigObject *src = NULL, *target = NULL;
        ObjectString *src_path = NULL, *target_path = NULL;
        QueueObject*  queue;
        bool disable_delete = false, enable_verbose = false, load_src = true;
        const char* arg_group[] = {"-d", "-v", NULL};
        const char* arg_along[] = {"--disable-delete",
                                   "--verbose",
                                   "--version",
                                   "--usage",
                                   "--help",
                                   NULL};

        if (argc == 1) {
                fprintf (stderr,
                         "main (): 错误: 需要参数。\n%s",
                         global_help[1]);
                return 1;
        }

        /*参数对象相关设置*/
        argobj = args_object_new ();
        args_object_register_type_along (argobj, arg_along);
        args_object_register_type_group (argobj, arg_group);
        args = args_object_pull_all_args (argobj, argc, argv);
        args_object_if_failed_then_return (argobj, args, 1);
        /*查看有无以下参数*/
        if (args_object_get_args (argobj, "--help")) {
                printf ("%s", global_help[1]);
                object_unref (argobj);
                return 0;
        }
        if (args_object_get_args (argobj, "--usage")) {
                printf ("%s", global_help[2]);
                object_unref (argobj);
                return 0;
        }
        if (args_object_get_args (argobj, "--version")) {
                printf ("%s", global_help[0]);
                object_unref (argobj);
                return 0;
        }
        if (args_object_get_args (argobj, "-d") ||
            args_object_get_args (argobj, "--disable-delete")) {
                disable_delete = true;
        }
        if (args_object_get_args (argobj, "--verbose"))
                enable_verbose = true;
        /*载入源目录和目标目录*/
        args = argobj->args_chain_head;
        while (args) {
                if (args->type == ARGS_TYPE_DEFAULT && load_src) {
                        src_path = object_string_reference (args->arg_head);
                        load_src = false;
                } else if (args->type == ARGS_TYPE_DEFAULT && !load_src) {
                        target_path = object_string_reference (args->arg_head);
                        break;
                }
                args = ARGS (object_node_get_next (OBJECT_NODE (args)));
        }
        object_unref (argobj);

        if (!src_path || !target_path) {
                fprintf (stderr,
                         "main (): 错误: 请指定源目录和目标目录的路径。\n");
                goto unexpectedly_exit;
        }

        src    = config_object_new ();
        target = config_object_new ();
        if (!config_object_set_path (src, src_path->charset, false)) {
                fprintf (stderr,
                         "main (): config_object_set_path (): 设置源路径失败: "
                         "%s\n",
                         src_path->charset);
                goto unexpectedly_exit;
        }
        if (!config_object_set_path (target, target_path->charset, true)) {
                fprintf (stderr,
                         "main (): config_object_set_path (): "
                         "设置目标路径失败: %s\n",
                         target_path->charset);
                goto unexpectedly_exit;
        }
        object_unref (src_path);
        object_unref (target_path);

        /*载入所有文件信息*/
        config_object_pull (src);
        config_object_pull (target);

        queue = config_object_compare_with (src, target, disable_delete);
        if (queue)
                queue_object_run_action (queue, enable_verbose);

        object_unref (src);
        object_unref (target);
        return 0;

unexpectedly_exit:
        object_unref (src_path);
        object_unref (target_path);
        object_unref (src);
        object_unref (target);
        return 1;
}