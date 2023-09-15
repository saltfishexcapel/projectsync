#include "object_v2-node.h"

void
object_node_init (ObjectNode* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, object_node_destory);
        obj->data       = NULL;
        obj->next       = NULL;
        obj->prev       = NULL;
        obj->end        = obj;
        obj->head       = NULL;
        obj->iter_flag  = NULL;
        obj->_dest_func = NULL;
}

ObjectNode*
object_node_new ()
{
        ObjectNode* obj;
        obj = OBJECT_NEW (ObjectNode);
        object_node_init (obj);
        return obj;
}

void
object_node_destruction (ObjectNode* obj)
{
        if (!obj)
                return;
        /*解引用下个节点*/
        object_unref (obj->next);
        if (obj->_dest_func && obj->data)
                obj->_dest_func (obj->data);
}

void
object_node_destory (ObjectNode* obj)
{
        if (!obj)
                return;
        object_node_destruction (obj);
        free (obj);
}

ObjectNode*
object_node_new_at_end (ObjectNode* obj_manager)
{
        if (!obj_manager)
                return NULL;
        object_node_set_new_end (obj_manager, object_node_new ());
        return obj_manager->end;
}

inline void
object_node_set_head (ObjectNode* obj, ObjectNode* obj_manager_head)
{
        if (obj && obj_manager_head)
                obj->head = obj_manager_head;
}

void
object_node_set_new_end (ObjectNode* obj_manager, ObjectNode* new_end_node)
{
        if (!obj_manager || !new_end_node) {
                if (new_end_node)
                        object_unref (new_end_node);
                return;
        }
        if (obj_manager == new_end_node)
                return;
        if (!obj_manager->end) {
                obj_manager->end = new_end_node;
                return;
        }
        object_node_set_prev (new_end_node, obj_manager->end);
        object_node_set_next (obj_manager->end, new_end_node);
        obj_manager->end = new_end_node;
        object_node_set_head (obj_manager->end, obj_manager);
}

void
object_node_set_as_manager (ObjectNode* obj)
{
        ObjectNode* tmp;
        if (!obj)
                return;
        object_node_set_head (obj, NULL);
        obj->end = obj;
        tmp      = object_node_get_next (obj);
        while (tmp) {
                object_node_set_head (tmp, obj);
                obj->end = tmp;
                tmp      = object_node_get_next (tmp);
        }
}

void
object_node_set_prev (ObjectNode* obj, ObjectNode* prev)
{
        if (obj)
                obj->prev = prev;
}

void
object_node_set_next (ObjectNode* obj, ObjectNode* next)
{
        if (obj)
                obj->next = next;
}

void
object_node_set_data (ObjectNode* obj, o_ptr data)
{
        if (obj)
                obj->data = data;
}

void
object_node_set_dest_func (ObjectNode* obj, o_dest_func _dest_func)
{
        if (obj)
                obj->_dest_func = _dest_func;
}

ObjectNode*
object_node_get_prev (ObjectNode* obj)
{
        if (obj)
                return obj->prev;
        return NULL;
}

ObjectNode*
object_node_get_next (ObjectNode* obj)
{
        if (obj)
                return obj->next;
        return NULL;
}

o_ptr
object_node_get_data (ObjectNode* obj)
{
        if (obj)
                return obj->data;
        return NULL;
}

void
object_node_connect_prev_next (ObjectNode* prev, ObjectNode* next)
{
        if (prev && next) {
                prev->next = next;
                /*next 被 prev 所需，故增加引用一次*/
                object_addref (next);
                next->prev = prev;
        } else if (!prev && next) {
                next->prev = NULL;
                /*next 也许被 System 所需，故增加引用一次*/
                object_addref (next);
                // object_addref (OBJECT (next));
        } else if (prev && !next)
                prev->next = NULL;
}

void
object_node_delete_node (ObjectNode* obj)
{
        if (!obj)
                return;
        /*如果是尾节点*/
        if (!obj->next) {
                /*设置尾节点为上一节点*/
                if (obj->head)
                        obj->head->end = obj->prev;
        }
        object_node_connect_prev_next (obj->prev, obj->next);
        object_unref (obj);
}

void
object_node_iter_set (ObjectNode* node)
{
        if (!node)
                return;
        node->iter_flag = node;
}

inline void
object_node_iter_reset (ObjectNode* node)
{
        object_node_iter_set (node);
}

ObjectNode*
object_node_iter_get (ObjectNode* node)
{
        ObjectNode* nd;
        if (!node)
                return NULL;
        nd              = (node->iter_flag == NULL ? NULL : node->iter_flag);
        node->iter_flag = object_node_get_next (node->iter_flag);
        return nd;
}
