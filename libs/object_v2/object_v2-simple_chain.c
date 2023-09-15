#include "object_v2-simple_chain.h"

#include "object_v2-string.h"

o_ptr
object_simple_chain_get_node (ObjectNode* head, const char* ckey)
{
        ObjectNode* tmp;
        if (!head || !ckey)
                return NULL;

        tmp = head;
        while (tmp != NULL) {
                if (object_string_compare_with_charset (
                            OBJECT_STRING (tmp->data),
                            ckey))
                        return (o_ptr)tmp;
                tmp = tmp->next;
        }
        return NULL;
}

o_ptr
object_simple_chain_get_vnode (ObjectNode* head, const char* vkey)
{
        ObjectNode* tmp;
        if (!head)
                return NULL;
        tmp = head;
        while (tmp != NULL) {
                if (O_PTR (vkey) == tmp->data)
                        return tmp;
                tmp = tmp->next;
        }
        return NULL;
}