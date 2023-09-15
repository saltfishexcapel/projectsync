#ifndef OBJECT_V2_NODE_H
#define OBJECT_V2_NODE_H

#include "object_v2-object.h"

/**
 * Node: 链表的单节点对象
 * 请注意，当销毁任意一个节点时，将会解引用下个节点。
 * 若要只摧毁当前节点，请先将前后节点相连，并增加后方节点的引用计数。
*/
OBJECT_DECLARE (ObjectNode, object_node)
struct _ObjectNode
{
        Object      parent;
        ObjectNode* prev;
        ObjectNode* next;
        ObjectNode* head;
        ObjectNode* end;
        ObjectNode* iter_flag;
        o_ptr       data;
        o_dest_func _dest_func;
};

#define OBJECT_NODE(any_obj) ((ObjectNode*)(any_obj))

/*设置 head 属性*/
void object_node_set_head (ObjectNode* obj, ObjectNode* obj_manager_head);
/*在 manager 的尾部创建一个新的节点对象*/
ObjectNode* object_node_new_at_end (ObjectNode* obj_manager);
/*只能用于设置新的尾节点*/
void object_node_set_new_end (ObjectNode* obj_manager, ObjectNode* new_end_node);
/*设置当前节点为管理节点，即控制链表表头与表尾的节点*/
void object_node_set_as_manager (ObjectNode* obj);

/*设置 prev 属性*/
void object_node_set_prev (ObjectNode* obj, ObjectNode* prev);
/*设置 next 属性*/
void object_node_set_next (ObjectNode* obj, ObjectNode* next);
/*设置 data 属性*/
void object_node_set_data (ObjectNode* obj, o_ptr data);
/*设置 data 的消除器*/
void object_node_set_dest_func (ObjectNode* obj, o_dest_func _dest_func);
ObjectNode* object_node_get_prev (ObjectNode* obj);
ObjectNode* object_node_get_next (ObjectNode* obj);
o_ptr       object_node_get_data (ObjectNode* obj);

/*会增加 next 的引用计数一次*/
void        object_node_connect_prev_next (ObjectNode* prev, ObjectNode* next);
/*删除头节点的话可能会出现一些问题*/
void        object_node_delete_node (ObjectNode* obj);

/*Node 的迭代方法，设置起始迭代位置*/
void object_node_iter_set (ObjectNode* node);
/*重置迭代位置到初始设置位置*/
void object_node_iter_reset (ObjectNode* node);
/*迭代一次*/
ObjectNode* object_node_iter_get (ObjectNode* node);

#endif