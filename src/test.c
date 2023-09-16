#include "ConfigObject.h"
#include <stdio.h>

int
main (int argc, const char* argv[])
{
        ConfigObject* src, *target;
        QueueObject* queue;

        src = config_object_new ();
        target = config_object_new ();
        config_object_set_path (src, argv[1]);
        config_object_set_path (target, argv[2]);
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