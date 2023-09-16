#include "ConfigObject.h"

#include <stdio.h>

int
main (int argc, const char* argv[])
{
        ConfigObject *src, *target;
        QueueObject*  queue;

        printf ("projsync v0.1 by 咸鱼雨涵/SaltFishExcapel\n");

        if (argc != 3) {
                printf ("\n用法: projsync [src] "
                        "[target]\n\tsrc:"
                        "\t需要同步的目录（必须存在）\n\ttarget:"
                        "\t目标目录（若不存在则将被创建）\n");
                return 1;
        }

        src    = config_object_new ();
        target = config_object_new ();
        if (!config_object_set_path (src, argv[1], false)) {
                fprintf (stderr,
                         "main (): config_object_set_path (): 设置源路径失败: "
                         "%s\n",
                         argv[1]);
                return 1;
        }
        if (!config_object_set_path (target, argv[2], true)) {
                fprintf (stderr,
                         "main (): config_object_set_path (): "
                         "设置目标路径失败: %s\n",
                         argv[2]);
                return 1;
        }
        config_object_pull (src);
        config_object_pull (target);

        queue = config_object_compare_with (src, target);
        if (queue)
                queue_object_run_action (queue);

        object_unref (queue);
        object_unref (src);
        object_unref (target);
        return 0;
}