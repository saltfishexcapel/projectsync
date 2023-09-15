#ifndef OBJECT_V2_HASH_H
#define OBJECT_V2_HASH_H

#include "object_v2-node.h"
#include "object_v2-object.h"
#include "object_v2-string.h"

/*重哈希所需达到的已用比率*/
#define OBJ_HASH_REHASH_RATE 0.75
/*收缩哈希的比率*/
#define OBJ_HASH_SHRINK_RATE 0.2
/*每次增加的哈希块大小*/
#define OBJ_HASH_CHUNK_SZ 512
/*每次可增加的哈希块大小最小值*/
#define OBJ_HASH_CHUNK_MIN 20
/*每次可增加的哈希块大小最大值*/
#define OBJ_HASH_CHUNK_MAX 10240

#if OBJ_HASH_CHUNK_SZ < OBJ_HASH_CHUNK_MIN || \
        OBJ_HASH_CHUNK_SZ > OBJ_HASH_CHUNK_MAX
        #error "哈希块默认增量设置不在可接受的范围内"
#endif

/**
 * Hash 的 KeyValue 对象
 */
OBJECT_DECLARE (ObjectHashKV, object_hash_kv)
struct _ObjectHashKV
{
        ObjectNode    parent;
        ObjectString* ckey;
        o_type8       vkey;
        o_ptr         value;
        o_dest_func   _vdest_func;
};

typedef ObjectHashKV* _object_hash_kv_ptr;
#define OBJECT_HASH_KV(any_obj) ((ObjectHashKV*)(any_obj))

/*设置 KeyValue 对象*/
void object_hash_kv_set_kv (ObjectHashKV* obj,
                            const char*   ckey,
                            bool          is_vkey,
                            o_ptr         value);
/*设置 value 的数据释放器，当 KeyValue 对象被销毁时会自动执行数据释放器*/
void object_hash_kv_set_vdest_func (ObjectHashKV* obj, o_dest_func _vdest_func);
/*获取 value*/
o_ptr object_hash_kv_get_value (ObjectHashKV* obj);

/**
 * 哈希表迭代对象
 */
typedef enum _ObjectHashIterState ObjectHashIterState;
enum _ObjectHashIterState
{
        OBJECT_HASH_ITER_STATE_START,
        OBJECT_HASH_ITER_STATE_DEFAULT,
        OBJECT_HASH_ITER_STATE_NULL,
        OBJECT_HASH_ITER_STATE_END
};

/**
 * 单哈希表对象
 */
OBJECT_DECLARE (ObjectHash, object_hash)
struct _ObjectHash
{
        Object               parent;         /*Object 父类*/
        int                  table_size;     /*表大小*/
        int                  table_old_size; /*旧表大小*/
        int                  table_used;     /*已用数量*/
        bool                 is_locked;      /*重哈希时上锁*/
        _object_hash_kv_ptr* main_table;     /*主存储表*/
        _object_hash_kv_ptr* cache_table;    /*临时挂载表*/
        o_uint               iter_num;        /*表序数*/
        ObjectHashKV*        current_iter;   /*当前迭代对象*/
        ObjectHashIterState  state;          /*当前迭代状态*/
};
#define OBJECT_HASH(any_obj) ((ObjectHash*)(any_obj))

/*根据指定键寻找对应值*/
o_ptr object_hash_get_value (ObjectHash* obj, const char* ckey, bool is_vkey);
/*根据键获取键值对对象*/
ObjectHashKV*
object_hash_get_kv (ObjectHash* obj, const char* ckey, bool is_vkey);
/*向哈希表添加一对键值对*/
void object_hash_set_value (ObjectHash* obj,
                            const char* ckey,
                            bool        is_vkey,
                            o_ptr       value,
                            o_dest_func _vdest_func);
/*删除一个键值对*/
void object_hash_dest_value (ObjectHash* obj, const char* ckey, bool is_vkey);
/*打印哈希表当前的所有数值分布以及使用量状态*/
void object_hash_print_table (ObjectHash* obj);

void object_hash_iter_reset (ObjectHash* obj);
// 迭代哈希表的一个元素，使用状态机进行迭代
ObjectHashKV* object_hash_iter_get (ObjectHash* obj);

#endif