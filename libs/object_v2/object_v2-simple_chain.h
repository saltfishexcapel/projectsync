#ifndef OBJECT_V2_SIMPLE_CHAIN_H
#define OBJECT_V2_SIMPLE_CHAIN_H

#include "object_v2-node.h"

/*ObjectNode 的抽象方法：根据 ckey 获取节点*/
o_ptr object_simple_chain_get_node (ObjectNode* head, const char* ckey);
/*ObjectNode 的抽象方法：根据 vkey 获取节点*/
o_ptr object_simple_chain_get_vnode (ObjectNode* head, const char* vkey);

#endif