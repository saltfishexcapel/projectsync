#ifndef ARGS_PARSER_H
#define ARGS_PARSER_H

#include "object_v2-hash.h"
#include "object_v2-node.h"
#include "object_v2-string.h"

#define ARGS_MAX_HEAD_LEN 100

typedef enum _ArgsType ArgsType;
enum _ArgsType
{
        ARGS_TYPE_ALONG,    // 单参数
        ARGS_TYPE_ONE,      // 一对一参数
        ARGS_TYPE_MORE,     // 一对多参数
        ARGS_TYPE_BUILD_IN, // 内嵌参数
        ARGS_TYPE_S_ALONG,  // -- 参数
        ARGS_TYPE_GROUP,    // 组合型参数，比如 -r -v -x => -rvx
        ARGS_TYPE_BODY,     // 参数体
};

typedef enum _ArgsObjectState ArgsObjectState;
enum _ArgsObjectState
{
        ARGS_STATE_DEFAULT,
        ARGS_STATE_ONE,       /*一对一参数的预备状态*/
        ARGS_STATE_MORE,      /*多参数的预备状态*/
        ARGS_STATE_MORE_WAIT, /*多参数状态进行中*/
        ARGS_STATE_FAILED,
        ARGS_STATE_STOP,
        ARGS_STATE_S_ALONG,
};

typedef enum _ArgsObjectEvent ArgsObjectEvent;
enum _ArgsObjectEvent
{
        ARGS_EVENT_STOP = 1,
        ARGS_EVENT_ALONG,
        ARGS_EVENT_ONE,
        ARGS_EVENT_MORE,
        ARGS_EVENT_BUILD_IN,
        ARGS_EVENT_S_ALONG,
        ARGS_EVENT_GROUP,
        ARGS_EVENT_DEFAULT
};

/**
 * 真正的应用级对象，保存一个或一组参数。
 * 参数对象由参数头和参数体组成，调用者先使用 type 判断类型，再使用 ObjectString
 * 的方法获得参数头和参数体，并做出相应的判断逻辑。
 */
OBJECT_DECLARE (Args, args)
#define ARGS(any) ((Args*)(any))
struct _Args
{
        ObjectNode parent_instance;
        ArgsType   type;
        /*参数头，如 -file, --path*/
        ObjectString* arg_head;
        /*参数体，如 filename.type, -Ibody1 -Ibody2
         * 等，较多的同类参数可存为参数体*/
        ObjectStrings* arg_body;
};

/*设置参数的参数头*/
void args_set_arg_head (Args* obj, const char* arg_head, ArgsType type);
/*设置参数的参数体，有多个 arg_body 可以重复使用*/
void args_set_arg_body (Args* obj, const char* arg_body);

/*参数类型注册对象*/
OBJECT_DECLARE (ArgTypeRegister, arg_type_register)
#define ARG_TYPE_REGISTER(any) ((ArgTypeRegister*)(any))
struct _ArgTypeRegister
{
        Object parent_instance;
        // ObjectString* arg_type_name;
        ArgsType type;
};

/**
 * 参数处理对象，处理三种不同的主程序传递参数：
 * 第一种：单参数，该参数头本身即代表数据，没有参数体。不属于任何一种参数头的归为单参数。
 * 第二种：一对一参数，该参数头只包含一个参数体，若未检测到参数体即报错。
 * 第三种：一对多参数，该参数头包含一个或多个参数体，若检测到了下一个参数头即停止读入。
 * 第四种：内嵌参数，按照单参数处理。
 */
OBJECT_DECLARE (ArgsObject, args_object)
#define ARGS_OBJECT(any) ((ArgsObject*)(any))
struct _ArgsObject
{
        Object parent_instance;
        /*用于存储参数链的数据结构*/
        Args*           args_chain_head;
        ObjectHash*     type_register;
        ArgsObjectState state;
        bool            flag_build_in;
};

/**
 * 注册各种参数头，字符串数组以 NULL 结尾
 * 如：
 *      static const char* along_type_args_register[] = {
 *              "-r",
 *              "-c",
 *              "-d",
 *              NULL
 *      };
 *
 *      args_object_register_type_along (obj, along_type_args_register);
 *
 */
void args_object_register_type (ArgsObject*  obj,
                                const char** arg_head_name,
                                ArgsType     type);
#define args_object_register_type_along(obj, arg_head_name) \
        args_object_register_type (obj, arg_head_name, ARGS_TYPE_ALONG)
#define args_object_register_type_one(obj, arg_head_name) \
        args_object_register_type (obj, arg_head_name, ARGS_TYPE_ONE)
#define args_object_register_type_more(obj, arg_head_name) \
        args_object_register_type (obj, arg_head_name, ARGS_TYPE_MORE)
#define args_object_register_type_build_in(obj, arg_head_name) \
        args_object_register_type (obj, arg_head_name, ARGS_TYPE_BUILD_IN)
#define args_object_register_type_group(obj, arg_head_name) \
        args_object_register_type (obj, arg_head_name, ARGS_TYPE_GROUP)

/*载入所有参数到参数对象，如果发生错误，就返回出错的那个参数*/
const Args*
args_object_pull_all_args (ArgsObject* obj, int argc, const char** arg);
/*简化版本失败返回*/
#define args_object_if_failed_then_return(args_object, args, ret)           \
        {                                                                   \
                if (args) {                                                 \
                        printf ("[WARING] 有参数载入出错！\n");             \
                        printf ("出错参数: %s\n", args->arg_head->charset); \
                        object_unref (OBJECT (args_object));                \
                        return ret;                                         \
                }                                                           \
        }
/**
 * @brief 获取一个可能存在的参数（若相同参数出现多次可能只取第一次出现的情况）
 */
const Args* args_object_get_args (ArgsObject* obj, const char* arg_name);

/**
 * 禁用 build-in 参数
 */
void args_object_disable_build_in (ArgsObject* obj);

#endif