#include "args_parser.h"

#include <stdio.h>
#include <string.h>

void
args_init (Args* obj)
{
        if (!obj)
                return;
        object_node_init (OBJECT_NODE (obj));
        object_set_destory_func (obj, args_destory);
        obj->arg_body = NULL;
        obj->arg_head = NULL;
        obj->type     = ARGS_TYPE_ALONG;
}

Args*
args_new ()
{
        Args* obj;
        obj = OBJECT_NEW (Args);
        args_init (obj);
        return obj;
}

void
args_destruction (Args* obj)
{
        if (!obj)
                return;
        /*解引用下一个节点*/
        object_unref (OBJECT_NODE (obj)->next);
        /*解除引用自身的数据*/
        object_unref (obj->arg_body);
        object_unref (obj->arg_head);
}

void
args_destory (Args* obj)
{
        if (!obj)
                return;
        args_destruction (obj);
        free (obj);
}

void
args_set_arg_head (Args* obj, const char* arg_head, ArgsType type)
{
        if (!obj || !arg_head)
                return;
        if (!obj->arg_head)
                obj->arg_head = object_string_new ();
        object_string_set_string (obj->arg_head, arg_head);
        obj->type = type;
}

void
args_set_arg_body (Args* obj, const char* arg_body)
{
        if (!obj || !arg_body)
                return;
        if (!obj->arg_body)
                obj->arg_body = object_strings_new ();
        object_strings_set_string (obj->arg_body, arg_body);
}

void
arg_type_register_init (ArgTypeRegister* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        
        object_set_destory_func (obj, arg_type_register_destory);
        obj->type = ARGS_TYPE_ALONG;
}

ArgTypeRegister*
arg_type_register_new ()
{
        ArgTypeRegister* obj;
        obj = OBJECT_NEW (ArgTypeRegister);
        arg_type_register_init (obj);
        return obj;
}

void
arg_type_register_destory (ArgTypeRegister* obj)
{
        if (!obj)
                return;
        free (obj);
}

void
args_object_init (ArgsObject* obj)
{
        const char* arg_s_along[] = {"--", NULL};
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, args_object_destory);
        obj->args_chain_head = NULL;
        obj->type_register   = object_hash_new ();
        obj->state           = ARGS_STATE_DEFAULT;
        obj->flag_build_in   = true;
        /*默认类型 -- */
        args_object_register_type (obj, arg_s_along, ARGS_TYPE_S_ALONG);
}

ArgsObject*
args_object_new ()
{
        ArgsObject* obj;
        obj = OBJECT_NEW (ArgsObject);
        args_object_init (obj);
        return obj;
}

void
args_object_destruction (ArgsObject* obj)
{
        if (!obj)
                return;
        object_unref (obj->args_chain_head);
        object_unref (obj->type_register);
}

void
args_object_destory (ArgsObject* obj)
{
        if (!obj)
                return;
        args_object_destruction (obj);
        free (obj);
}

void
args_object_register_type (ArgsObject*  obj,
                           const char** arg_head_name,
                           ArgsType     type)
{
        ArgTypeRegister* reg;
        if (!obj || !arg_head_name)
                return;
        for (int i = 0; arg_head_name[i] != NULL; ++i) {
                reg       = arg_type_register_new ();
                reg->type = type;
                object_hash_set_value (
                        obj->type_register,
                        arg_head_name[i],
                        false,
                        (o_ptr)reg,
                        OBJECT_DEST_FUNC (arg_type_register_destory));
        }
}

/**
 * 从参数获取一个事件类型
 */
static ArgsObjectEvent
args_object_catch_event (const char* arg_name, ArgTypeRegister* reg)
{
        if (!arg_name)
                return ARGS_EVENT_STOP;
        if (!reg)
                return ARGS_EVENT_DEFAULT;
        switch (reg->type) {
        case ARGS_TYPE_ALONG:
                return ARGS_EVENT_ALONG;

        case ARGS_TYPE_ONE:
                return ARGS_EVENT_ONE;

        case ARGS_TYPE_MORE:
                return ARGS_EVENT_MORE;

        case ARGS_TYPE_BUILD_IN:
                return ARGS_EVENT_BUILD_IN;

        case ARGS_TYPE_S_ALONG:
                return ARGS_EVENT_S_ALONG;

        case ARGS_TYPE_GROUP:
                return ARGS_EVENT_GROUP;

        default:
                return ARGS_TYPE_ALONG;
        }
}

/**
 * 改变对象状态为 state
 */
static inline void
args_object_change_state (ArgsObject* obj, ArgsObjectState state)
{
        obj->state = state;
}

/**
 * 判断该 arg_name 是否包含等于号，即判断该名称是否为 build-in 参数
 * 返回等于符号的位置指针
 */
static inline const char*
args_object_is_build_in_args (const char* arg_name)
{
        return strchr (arg_name, '=');
}

/**
 * 获取 build-in 参数的参数头
 */
static const char*
args_object_get_build_in_head (const char* arg_name, char* pulls)
{
        char*       buf;
        const char* equal;
        static char _buffer[100];
        int         build_in_len;
        if (pulls)
                buf = pulls;
        else
                buf = _buffer;
        equal = args_object_is_build_in_args (arg_name);
        if (!equal)
                return NULL;

        build_in_len = INT (equal - arg_name + 1);
        if (build_in_len >= ARGS_MAX_HEAD_LEN) {
                fprintf (stderr,
                         "[ERROR]: 在 %s:%s, "
                         "<%d>行，参数长度大于了默认长度。\n",
                         __FILE__,
                         __FUNCTION__,
                         __LINE__);
                return NULL;
        }
        strncpy (buf, arg_name, build_in_len);
        buf[build_in_len] = '\0';

        return buf;
}

static bool
args_object_is_group_head (ArgsObject* obj, const char* arg_name)
{
        ArgTypeRegister* reg;
        char             test[3] = {'-', '\0', '\0'};
        if (!obj || !arg_name || arg_name[0] != '-' || arg_name[1] == '\0')
                return false;
        test[1] = arg_name[1];
        reg     = ARG_TYPE_REGISTER (
                object_hash_get_value (obj->type_register, test, false));
        if (reg && reg->type == ARGS_TYPE_GROUP)
                return true;
        return false;
}

/**
 * 为 ArgsObject 添加一对新的参数
 */
static void
args_object_add_node (ArgsObject* obj, const char* arg_name, ArgsType type)
{
        Args* new_args;

        if (obj->args_chain_head) {
                new_args = args_new ();
                args_set_arg_head (new_args, arg_name, type);
                object_node_set_new_end (OBJECT_NODE (obj->args_chain_head),
                                         OBJECT_NODE (new_args));
                return;
        }
        obj->args_chain_head = args_new ();
        object_node_set_as_manager (OBJECT_NODE (obj->args_chain_head));
        args_set_arg_head (obj->args_chain_head, arg_name, type);
}

static void
args_object_action_for_event_along (ArgsObject* obj, const char* arg_name)
{
        args_object_change_state (obj, ARGS_STATE_DEFAULT);
        args_object_add_node (obj, arg_name, ARGS_TYPE_ALONG);
}

static void
args_object_action_for_event_one (ArgsObject* obj, const char* arg_name)
{
        args_object_change_state (obj, ARGS_STATE_ONE);
        args_object_add_node (obj, arg_name, ARGS_TYPE_ONE);
}

static bool
args_object_action_for_event_build_in (ArgsObject* obj, const char* arg_name)
{
        const char *arg_head, *arg_body;
        arg_body = args_object_is_build_in_args (arg_name);
        if (!arg_body || *arg_body == '\0' || *(arg_body + 1) == '\0') {
                args_object_change_state (obj, ARGS_STATE_FAILED);
                args_object_add_node (obj, arg_name, ARGS_TYPE_BUILD_IN);
                return false;
        }
        args_object_change_state (obj, ARGS_STATE_DEFAULT);
        arg_head = args_object_get_build_in_head (arg_name, NULL);
        args_object_add_node (obj, arg_head, ARGS_TYPE_BUILD_IN);
        args_set_arg_body (ARGS (OBJECT_NODE (obj->args_chain_head)->end),
                           (arg_body + 1));
        return true;
}

static bool
args_object_action_for_event_group (ArgsObject* obj, const char* arg_name)
{
        char test[3] = {'-', '\0', '\0'};
        for (int i = 1; arg_name[i] != '\0'; ++i) {
                test[1] = arg_name[i];
                args_object_add_node (obj, test, ARGS_TYPE_GROUP);
                if (!object_hash_get_kv (obj->type_register, test, false)) {
                        args_object_change_state (obj, ARGS_STATE_FAILED);
                        return false;
                }
        }
        args_object_change_state (obj, ARGS_STATE_DEFAULT);
        return true;
}

static void
args_object_action_for_event_more (ArgsObject* obj, const char* arg_name)
{
        args_object_change_state (obj, ARGS_STATE_MORE);
        args_object_add_node (obj, arg_name, ARGS_TYPE_MORE);
}

static void
args_object_action_for_event_s_along (ArgsObject* obj, const char* arg_name)
{
        args_object_change_state (obj, ARGS_STATE_S_ALONG);
        args_object_add_node (obj, arg_name, ARGS_TYPE_S_ALONG);
}

static Args*
args_object_action_for_state_default (ArgsObject*     obj,
                                      const char*     arg_name,
                                      ArgsObjectEvent event)
{
        bool is_success;
        switch (event) {
        case ARGS_EVENT_DEFAULT:
        case ARGS_EVENT_ALONG:
                args_object_action_for_event_along (obj, arg_name);
                return NULL;
        case ARGS_EVENT_BUILD_IN: {
                is_success =
                        args_object_action_for_event_build_in (obj, arg_name);
                if (is_success)
                        return NULL;
                else
                        return ARGS (OBJECT_NODE (obj->args_chain_head)->end);
        }
        case ARGS_EVENT_GROUP: {
                is_success = args_object_action_for_event_group (obj, arg_name);
                if (is_success)
                        return NULL;
                else
                        return ARGS (OBJECT_NODE (obj->args_chain_head)->end);
        }
        case ARGS_EVENT_MORE:
                args_object_action_for_event_more (obj, arg_name);
                return NULL;
        case ARGS_EVENT_ONE:
                args_object_action_for_event_one (obj, arg_name);
                return NULL;
        case ARGS_EVENT_S_ALONG:
                args_object_action_for_event_s_along (obj, arg_name);
                return NULL;
        default: /*ARGS_EVENT_STOP*/
                args_object_change_state (obj, ARGS_STATE_STOP);
                return NULL;
        }
}

static Args*
args_object_action_for_state_more_wait (ArgsObject*     obj,
                                        const char*     arg_name,
                                        ArgsObjectEvent event)
{
        bool is_success;
        if (event == ARGS_EVENT_STOP) {
                args_object_change_state (obj, ARGS_STATE_STOP);
                return NULL;
        }
        switch (event) {
        case ARGS_EVENT_ALONG:
                args_object_action_for_event_along (obj, arg_name);
                return NULL;
        case ARGS_EVENT_BUILD_IN: {
                is_success =
                        args_object_action_for_event_build_in (obj, arg_name);
                if (is_success)
                        return NULL;
                else
                        return ARGS (OBJECT_NODE (obj->args_chain_head)->end);
        }
        case ARGS_EVENT_GROUP: {
                is_success = args_object_action_for_event_group (obj, arg_name);
                if (is_success)
                        return NULL;
                else
                        return ARGS (OBJECT_NODE (obj->args_chain_head)->end);
        }
        case ARGS_EVENT_MORE:
                args_object_action_for_event_more (obj, arg_name);
                return NULL;
        case ARGS_EVENT_ONE:
                args_object_action_for_event_one (obj, arg_name);
                return NULL;
        case ARGS_EVENT_STOP:
                args_object_change_state (obj, ARGS_STATE_STOP);
                return NULL;
        default: /*ARGS_EVENT_DEFAULT*/
                /*ARGS_EVENT_S_ALONG*/
                args_set_arg_body (
                        ARGS (OBJECT_NODE (obj->args_chain_head)->end),
                        arg_name);
                return NULL;
        }
}

static Args*
args_object_action_for_state_more (ArgsObject*     obj,
                                   const char*     arg_name,
                                   ArgsObjectEvent event)
{
        ObjectNode* node;

        node = OBJECT_NODE (obj->args_chain_head)->end;
        args_set_arg_body (ARGS (node), arg_name);
        if (event != ARGS_EVENT_DEFAULT && event != ARGS_EVENT_S_ALONG &&
            event != ARGS_EVENT_GROUP) {
                args_object_change_state (obj, ARGS_STATE_FAILED);
                return ARGS (node);
        }
        args_object_change_state (obj, ARGS_STATE_MORE_WAIT);
        return NULL;
}

static Args*
args_object_action_for_state_one (ArgsObject*     obj,
                                  const char*     arg_name,
                                  ArgsObjectEvent event)
{
        ObjectNode* node;

        node = OBJECT_NODE (obj->args_chain_head)->end;
        args_set_arg_body (ARGS (node), arg_name);
        if (event != ARGS_EVENT_DEFAULT && event != ARGS_EVENT_S_ALONG &&
            event != ARGS_EVENT_GROUP) {
                args_object_change_state (obj, ARGS_STATE_FAILED);
                return ARGS (node);
        }
        args_object_change_state (obj, ARGS_STATE_DEFAULT);
        return NULL;
}

static Args*
args_object_action_for_state_s_along (ArgsObject*     obj,
                                      const char*     arg_name,
                                      ArgsObjectEvent event)
{
        if (event == ARGS_EVENT_STOP) {
                args_object_change_state (obj, ARGS_STATE_STOP);
                return NULL;
        }

        args_object_add_node (obj, arg_name, ARGS_TYPE_ALONG);
        return NULL;
}

/**
 * 是一个状态机。
 * 事件列表：
 *      序号    arg_name   reg          事件
 *      1       NULL       any          停止
 *      2       *          ALONG        单参数
 *      3       *          ONE          一对一参数
 *      4       *          MORE         多参数
 *      5       *          BUILD_IN     内嵌参数
 *      6       *          NULL         默认事件
 */
static Args*
args_object_pull_all_args_once (ArgsObject*      obj,
                                const char*      arg_name,
                                ArgTypeRegister* reg)
{
        ArgsObjectEvent event;
        static Args*    args = NULL;

        event                = args_object_catch_event (arg_name, reg);

        switch (obj->state) {
        case ARGS_STATE_DEFAULT:
                args = args_object_action_for_state_default (obj,
                                                             arg_name,
                                                             event);
                break;
        case ARGS_STATE_FAILED:
                return args;
        case ARGS_STATE_MORE:
                args = args_object_action_for_state_more (obj, arg_name, event);
                break;
        case ARGS_STATE_MORE_WAIT:
                args = args_object_action_for_state_more_wait (obj,
                                                               arg_name,
                                                               event);
                break;
        case ARGS_STATE_ONE:
                args = args_object_action_for_state_one (obj, arg_name, event);
                break;
        case ARGS_STATE_S_ALONG:
                args = args_object_action_for_state_s_along (obj,
                                                             arg_name,
                                                             event);
                break;
        default: /*ARGS_STATE_STOP*/
                break;
        }
        return args;
}

/**
 * 根据参数名获取一个注册类型对象
 */
static ArgTypeRegister*
args_object_get_register (ArgsObject* obj, const char* arg_name)
{
        static ArgTypeRegister tgroup;
        const char*            arg;
        if (!obj || !arg_name)
                return NULL;
        if (obj->flag_build_in &&
            (arg = args_object_get_build_in_head (arg_name, NULL))) {
                return ARG_TYPE_REGISTER (
                        object_hash_get_value (obj->type_register, arg, false));
        }
        if (args_object_is_group_head (obj, arg_name)) {
                object_init (OBJECT (&tgroup));
                tgroup.type = ARGS_TYPE_GROUP;
                return &tgroup;
        }
        return ARG_TYPE_REGISTER (
                object_hash_get_value (obj->type_register, arg_name, false));
}

const Args*
args_object_pull_all_args (ArgsObject* obj, int argc, const char** argv)
{
        Args*            ret;
        ArgTypeRegister* reg;
        if (!obj)
                return NULL;
        for (int i = 1; i < argc; ++i) {
                reg = args_object_get_register (obj, argv[i]);
                /*将值一次次地交给 pull_all_args_once，状态机调用*/
                ret = args_object_pull_all_args_once (obj, argv[i], reg);
                if (ret)
                        return ret;
        }
        /*状态机结束调用*/
        return args_object_pull_all_args_once (obj, NULL, reg);
}

const Args*
args_object_get_args (ArgsObject* obj, const char* arg_name)
{
        ObjectNode* node;
        if (!obj || !arg_name)
                return NULL;
        node = OBJECT_NODE (obj->args_chain_head);
        while (node) {
                if (object_string_compare_with_charset (ARGS (node)->arg_head,
                                                        arg_name)) {
                        return ARGS (node);
                }
                node = object_node_get_next (node);
        }
        return NULL;
}

void
args_object_disable_build_in (ArgsObject* obj)
{
        if (obj)
                obj->flag_build_in = false;
}