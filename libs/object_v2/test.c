#include "args_parser.h"

#include <stdio.h>

int
main (int argc, const char** argv)
{
        const Args*    args;
        ObjectStrings* strs;
        const char*    arg_along[]    = {"-Wall", NULL};
        const char*    arg_one[]      = {"-o", "-i", NULL};
        const char*    arg_more[]     = {"--files", "--puts", NULL};
        const char*    arg_build_in[] = {"--builds=", "--target=", NULL};
        const char*    arg_group[]    = {"-a", "-b", "-c", NULL};
        ArgsObject*    obj            = args_object_new ();

        args_object_register_type_along (obj, arg_along);
        args_object_register_type_one (obj, arg_one);
        args_object_register_type_more (obj, arg_more);
        args_object_register_type_build_in (obj, arg_build_in);
        args_object_register_type_group (obj, arg_group);

        //args_object_disable_build_in (obj);

        args = args_object_pull_all_args (obj, argc, argv);
        /*判断是否返回出错，并报出出错参数信息*/
        args_object_if_failed_then_return (obj, args, 1);

        args = obj->args_chain_head;
        while (args) {
                printf ("type<%d>, ->", args->type);
                object_string_println (args->arg_head);
                strs = args->arg_body;
                while (strs) {
                        printf ("string<%p>, ->", strs->string);
                        object_string_println (strs->string);
                        strs = OBJECT_STRINGS (
                                object_node_get_next (OBJECT_NODE (strs)));
                }
                args = ARGS (object_node_get_next (OBJECT_NODE (args)));
        }

        // args = args_object_get_args (obj, "--puts");
        // if (args)
        // printf ("检测到了 --puts 参数\n");

        object_unref (obj);
        return 0;
}
