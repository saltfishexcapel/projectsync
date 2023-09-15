#include "object_v2-string.h"

#include <stdio.h>
#include <string.h>

void
object_string_init (ObjectString* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, object_string_destory);
        obj->length  = 0;
        obj->charset = NULL;
        obj->cache   = NULL;
}

ObjectString*
object_string_new ()
{
        ObjectString* obj;
        obj = OBJECT_NEW (ObjectString);
        object_string_init (obj);
        return obj;
}

void
object_string_destruction (ObjectString* obj)
{
        if (obj && obj->charset)
                free (obj->charset);
}

void
object_string_destory (ObjectString* obj)
{
        if (obj) {
                object_string_destruction (obj);
                free (obj);
        }
}

ObjectString*
object_string_new_with (const char* str)
{
        ObjectString* obj;
        obj = object_string_new ();
        object_string_set_string (obj, str);
        return obj;
}

void
object_string_set_string (ObjectString* obj, const char* str)
{
        if (!obj || !str)
                return;
        if (obj->charset)
                free (obj->charset);
        obj->length  = strlen (str);
        obj->charset = (char*)malloc (obj->length + 1);
        strcpy (obj->charset, str);
}

const char*
object_string_get_string (ObjectString* obj)
{
        if (obj)
                return obj->charset;
        else
                return NULL;
}

void
object_string_append_string (ObjectString* obj, const char* add_str)
{
        if (!obj || !add_str)
                return;
        if (!obj->charset) {
                object_string_set_string (obj, add_str);
                return;
        }
        obj->length += strlen (add_str);
        obj->cache = (char*)malloc (obj->length + 1);
        strcpy (obj->cache, obj->charset);
        strcat (obj->cache, add_str);
        free (obj->charset);
        obj->charset = obj->cache;
}

void
object_string_println (ObjectString* obj)
{
        if (!obj)
                return;
        if (!obj->charset)
                printf ("<string, %p>:NULL\n", obj);
        else
                printf ("%s\n", obj->charset);
}

void
object_string_getln (ObjectString* obj)
{
        char*       tmp;
        static char getln_buffer[2048];

        if (!obj)
                return;

        tmp = getln_buffer;
        fgets (getln_buffer, 2047, stdin);
        while (*tmp != '\n')
                ++tmp;
        *tmp = '\0';
        object_string_set_string (obj, getln_buffer);
}

bool
object_string_compare (const ObjectString* obja, const ObjectString* objb)
{
        if (!obja || !objb)
                return false;
        if (obja->length == 0 && objb->length == 0)
                return true;
        if (obja->length != objb->length)
                return false;
        if (!strcmp (obja->charset, objb->charset))
                return true;
        else
                return false;
}

bool
object_string_compare_with_charset (const ObjectString* obj,
                                    const char*         charset)
{
        if (!obj || !charset)
                return false;
        if (!strcmp (obj->charset, charset))
                return true;
        return false;
}

void
object_string_copy_to_charset (ObjectString* obj, char** charset)
{
        if (!obj || !charset)
                return;
        *charset = (char*)malloc ((obj->length + 1) * sizeof (char));
        strcpy (*charset, object_string_get_string (obj));
}

inline ObjectString*
object_string_reference (ObjectString* obj)
{
        return object_reference_to (obj, OBJECT_STRING);
}

static bool
object_strings_is_head (ObjectStrings* obj)
{
        if (!obj)
                return false;
        if (!object_node_get_prev (OBJECT_NODE (obj)))
                return true;
        else
                return false;
}

void
object_strings_init (ObjectStrings* obj)
{
        if (!obj)
                return;
        object_node_init (OBJECT_NODE (obj));
        object_set_destory_func (obj, object_strings_destory);
        obj->string = NULL;
}

ObjectStrings*
object_strings_new ()
{
        ObjectStrings* obj;
        obj = OBJECT_NEW (ObjectStrings);
        object_strings_init (obj);
        return obj;
}

void
object_strings_destruction (ObjectStrings* obj)
{
        if (!obj)
                return;
        /*解引用下个节点*/
        object_unref (OBJECT_NODE (obj)->next);
        /*解引用自身数据*/
        object_unref (obj->string);
}

void
object_strings_destory (ObjectStrings* obj)
{
        if (!obj)
                return;
        object_strings_destruction (obj);
        free (obj);
}

void
object_strings_set_string (ObjectStrings* strings_manager,
                           const char*    set_charset)
{
        bool           is_find = false;
        ObjectStrings* str;
        if (!strings_manager || !set_charset)
                return;
        if (!strings_manager->string) {
                strings_manager->string = object_string_new ();
                object_string_set_string (strings_manager->string, set_charset);
                return;
        }
        str = strings_manager;
        while (str) {
                if (object_string_compare_with_charset (str->string,
                                                        set_charset)) {
                        is_find = true;
                        break;
                }
                str = OBJECT_STRINGS (OBJECT_NODE (str)->next);
        }
        /*寻找到了已存在的 String*/
        if (is_find) {
                /*则修改该 String 的值*/
                object_string_set_string (str->string, set_charset);
                return;
        }
        /*否则则添加一条新 Strings 对象*/
        str         = object_strings_new ();
        str->string = object_string_new ();
        object_string_set_string (str->string, set_charset);
        object_node_set_new_end (OBJECT_NODE (strings_manager),
                                 OBJECT_NODE (str));
}

void
object_strings_dest_string (ObjectStrings*  strings_manager,
                            const char*     dest_charset,
                            ObjectStrings** manager_var)
{
        bool           is_find = false;
        ObjectStrings* str;
        if (!strings_manager || !dest_charset || !manager_var)
                return;
        str = strings_manager;
        while (str) {
                if (object_string_compare_with_charset (str->string,
                                                        dest_charset)) {
                        is_find = true;
                        break;
                }
                str = OBJECT_STRINGS (object_node_get_next (OBJECT_NODE (str)));
        }
        /*寻找到了已存在的 String*/
        if (is_find) {
                /*如果不是头节点则直接调用节点删除方法*/
                if (!object_strings_is_head (str)) {
                        object_node_delete_node (OBJECT_NODE (str));
                } else {
                        *manager_var =
                                object_reference_to (OBJECT_NODE (str)->next,
                                                     OBJECT_STRINGS);
                        object_node_set_as_manager (OBJECT_NODE (*manager_var));
                        object_node_set_prev (OBJECT_NODE (str)->next, NULL);
                        object_unref (str);
                }
                return;
        }
}

void
object_strings_iter_set (ObjectStrings* strings)
{
        object_node_iter_set (OBJECT_NODE (strings));
}

void
object_strings_iter_reset (ObjectStrings* strings)
{
        object_node_iter_reset (OBJECT_NODE (strings));
}

const char*
object_strings_iter_get (ObjectStrings* strings)
{
        ObjectStrings* str;
        str = OBJECT_STRINGS (object_node_iter_get (OBJECT_NODE (strings)));
        if (str)
                return object_string_get_string (str->string);
        else
                return NULL;
}