#include "object_v2-hash.h"

#include "object_v2-simple_chain.h"
#include "object_v2-string.h"

#include <stdio.h>
#include <string.h>

void
object_hash_kv_init (ObjectHashKV* obj)
{
        if (!obj)
                return;
        object_node_init (OBJECT_NODE (obj));
        object_set_destory_func (obj, object_hash_kv_destory);
        obj->ckey        = NULL;
        obj->value       = NULL;
        obj->_vdest_func = NULL;
        obj->vkey        = 0;
}

ObjectHashKV*
object_hash_kv_new ()
{
        ObjectHashKV* obj;
        obj = OBJECT_NEW (ObjectHashKV);
        object_hash_kv_init (obj);
        return obj;
}

void
object_hash_kv_destruction (ObjectHashKV* obj)
{
        if (!obj)
                return;
        /*解引用下个节点*/
        object_unref (OBJECT_NODE (obj)->next);
        /*解引用 String 键对象*/
        object_unref (obj->ckey);
        if (obj->_vdest_func)
                obj->_vdest_func (obj->value);
}

void
object_hash_kv_destory (ObjectHashKV* obj)
{
        if (!obj)
                return;
        object_hash_kv_destruction (obj);
        free (obj);
}

void
object_hash_kv_set_kv (ObjectHashKV* obj,
                       const char*   ckey,
                       bool          is_vkey,
                       o_ptr         value)
{
        if (!obj || !ckey)
                return;
        if (is_vkey) {
                if (obj->ckey)
                        object_unref (obj->ckey);
                obj->ckey = NULL;
                obj->vkey = (o_type8)(ckey);
                object_node_set_data (OBJECT_NODE (obj), O_PTR (obj->vkey));
        } else {
                obj->vkey = 0;
                if (!obj->ckey)
                        obj->ckey = object_string_new ();
                object_string_set_string (obj->ckey, ckey);
                object_node_set_data (OBJECT_NODE (obj), obj->ckey);
        }
        if (obj->_vdest_func && obj->value)
                obj->_vdest_func (obj->value);
        obj->value = value;
}

void
object_hash_kv_set_vdest_func (ObjectHashKV* obj, o_dest_func _vdest_func)
{
        if (!obj || !_vdest_func)
                return;
        obj->_vdest_func = _vdest_func;
}

inline o_ptr
object_hash_kv_get_value (ObjectHashKV* obj)
{
        if (obj)
                return obj->value;
        return NULL;
}

void
object_hash_iter_reset (ObjectHash* obj)
{
        if (!obj || !obj->main_table)
                return;
        obj->current_iter = NULL;
        obj->iter_num      = 0;
        obj->state        = OBJECT_HASH_ITER_STATE_START;
}

ObjectHashKV*
object_hash_iter_get (ObjectHash* obj)
{
        if (!obj || obj->table_size == 0)
                return NULL;

retry:
        switch (obj->state) {
        case OBJECT_HASH_ITER_STATE_START: {
                obj->current_iter = obj->main_table[0];
                break;
        }
        case OBJECT_HASH_ITER_STATE_DEFAULT: {
                obj->current_iter = OBJECT_HASH_KV (
                        object_node_get_next (OBJECT_NODE (obj->current_iter)));
                break;
        }
        case OBJECT_HASH_ITER_STATE_NULL: {
                obj->iter_num += 1;
                obj->current_iter = obj->main_table[obj->iter_num];
                break;
        }
        case OBJECT_HASH_ITER_STATE_END:
                obj->current_iter = NULL;
                break;
        }

        if (obj->current_iter) {
                obj->state = OBJECT_HASH_ITER_STATE_DEFAULT;
                return obj->current_iter;
        } else if (obj->iter_num == obj->table_size) {
                obj->state = OBJECT_HASH_ITER_STATE_END;
                return NULL;
        } else {
                obj->state = OBJECT_HASH_ITER_STATE_NULL;
                goto retry;
        }
}

void
object_hash_init (ObjectHash* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, object_hash_destory);
        obj->cache_table    = NULL;
        obj->main_table     = NULL;
        obj->table_size     = 0;
        obj->table_used     = 0;
        obj->table_old_size = 0;
        obj->is_locked      = false;
}

ObjectHash*
object_hash_new ()
{
        ObjectHash* obj;
        obj = OBJECT_NEW (ObjectHash);
        object_hash_init (obj);
        return obj;
}

void
object_hash_destruction (ObjectHash* obj)
{
        if (!obj)
                return;
        for (int i = 0; i < obj->table_size; ++i)
                object_unref (obj->main_table[i]);
        free (obj->main_table);
}

void
object_hash_destory (ObjectHash* obj)
{
        if (!obj)
                return;
        object_hash_destruction (obj);
        free (obj);
}

static o_uint
object_hash (const char* ckey, bool is_vkey, o_uint table_size)
{
        int     klen;
        o_ulong chk_sum = 17;

        if ((!ckey && !is_vkey) || table_size == 0)
                return 0;
        if (is_vkey)
                return (o_uint)((o_ulong)(ckey) % table_size);
        klen = strlen (ckey);
        for (int i = 0; i < klen; ++i)
                chk_sum = chk_sum * 73 + (o_ulong)(ckey[i]);
        return (o_uint)(chk_sum % table_size);
}

static void
object_hash_set_value_once (_object_hash_kv_ptr* table,
                            const char*          ckey,
                            bool                 is_vkey,
                            o_ptr                value,
                            o_dest_func          _dest_func,
                            int                  table_size)
{
        o_uint        magic;
        ObjectHashKV* new_kv_node;
        if (!table)
                return;
        magic = object_hash (ckey, is_vkey, table_size);
        if (!table[magic]) {
                table[magic] = object_hash_kv_new ();
                object_hash_kv_set_kv (table[magic], ckey, is_vkey, value);
                object_hash_kv_set_vdest_func (table[magic], _dest_func);
                return;
        }
        new_kv_node = object_hash_kv_new ();
        object_hash_kv_set_kv (new_kv_node, ckey, is_vkey, value);
        object_hash_kv_set_vdest_func (new_kv_node, _dest_func);
        object_node_set_new_end (OBJECT_NODE (table[magic]),
                                 OBJECT_NODE (new_kv_node));
}

static void
object_hash_rehash (ObjectHash* obj)
{
        ObjectHashKV *       tmp, *tmp1;
        _object_hash_kv_ptr* old_table;

        /*第一次分配表*/
        if (obj->main_table == NULL) {
                obj->cache_table = (_object_hash_kv_ptr*)malloc (
                        sizeof (_object_hash_kv_ptr) * obj->table_size);
                memset (obj->cache_table,
                        0,
                        sizeof (_object_hash_kv_ptr) * obj->table_size);
                if (!obj->cache_table) {
                        fprintf (stderr,
                                 "严重错误！object_v2-hash，rehash "
                                 "时无法分配新的大小。\n");
                        return;
                }
                obj->main_table     = obj->cache_table;
                obj->table_old_size = obj->table_size;
                obj->is_locked      = false;
                return;
        }

        obj->cache_table = (_object_hash_kv_ptr*)malloc (
                sizeof (_object_hash_kv_ptr) * obj->table_size);
        memset (obj->cache_table,
                0,
                sizeof (_object_hash_kv_ptr) * obj->table_size);
        /*重哈希表数据*/
        for (int i = 0; i < obj->table_old_size; ++i) {
                tmp = obj->main_table[i];
                if (!tmp)
                        continue;
                while (tmp) {
                        object_hash_set_value_once (
                                obj->cache_table,
                                object_string_get_string (tmp->ckey),
                                (tmp->vkey != 0),
                                tmp->value,
                                tmp->_vdest_func,
                                obj->table_size);
                        tmp = OBJECT_HASH_KV (
                                object_node_get_next (OBJECT_NODE (tmp)));
                }
        }
        old_table = obj->main_table;
        /*切换表*/
        obj->main_table = obj->cache_table;
        /*释放原表*/
        for (int i = 0; i < obj->table_old_size; ++i) {
                tmp = old_table[i];
                if (!tmp)
                        continue;
                while (tmp) {
                        object_unref (tmp->ckey);
                        tmp1 = tmp;
                        tmp  = OBJECT_HASH_KV (
                                object_node_get_next (OBJECT_NODE (tmp)));
                        free (tmp1);
                }
        }
        free (old_table);
        obj->is_locked = false;
}

static void
object_hash_add_size (ObjectHash* obj, o_uint add_size)
{
        if (!obj || add_size < OBJ_HASH_CHUNK_MIN ||
            add_size > OBJ_HASH_CHUNK_MAX)
                return;
        obj->is_locked      = true;
        obj->table_old_size = obj->table_size;
        obj->table_size += add_size;
        object_hash_rehash (obj);
}

static void
object_hash_reduce_size (ObjectHash* obj, o_uint reduce_size)
{
        if (!obj || (obj->table_size - reduce_size < OBJ_HASH_CHUNK_MIN))
                return;
        obj->is_locked = true;
        obj->table_size -= reduce_size;
        object_hash_rehash (obj);
}

o_ptr
object_hash_get_value (ObjectHash* obj, const char* ckey, bool is_vkey)
{
        ObjectHashKV* find_node;

        if (!obj)
                return NULL;
        find_node = object_hash_get_kv (obj, ckey, is_vkey);
        return object_hash_kv_get_value (find_node);
}

ObjectHashKV*
object_hash_get_kv (ObjectHash* obj, const char* ckey, bool is_vkey)
{
        o_uint        magic;
        ObjectHashKV* find_node;

        if (!obj || (!is_vkey && !ckey))
                return NULL;
        if (obj->table_size == 0)
                return NULL;
        magic = object_hash (ckey, is_vkey, obj->table_size);
        if (is_vkey) {
                find_node = OBJECT_HASH_KV (object_simple_chain_get_vnode (
                        OBJECT_NODE (obj->main_table[magic]),
                        ckey));
        } else {
                find_node = OBJECT_HASH_KV (object_simple_chain_get_node (
                        OBJECT_NODE (obj->main_table[magic]),
                        ckey));
        }
        return find_node;
}

static void
object_hash_wait_unlock (ObjectHash* obj)
{
        while (obj->is_locked) {
        }
}

void
object_hash_set_value (ObjectHash* obj,
                       const char* ckey,
                       bool        is_vkey,
                       o_ptr       value,
                       o_dest_func _vdest_func)
{
        ObjectHashKV* exist;

        if (!obj || (!is_vkey && !ckey))
                return;
        object_hash_wait_unlock (obj);
        if (obj->table_size == 0)
                object_hash_add_size (obj, OBJ_HASH_CHUNK_SZ);
        exist = object_hash_get_kv (obj, ckey, is_vkey);
        if (exist) {
                object_hash_kv_set_kv (exist, ckey, is_vkey, value);
                object_hash_kv_set_vdest_func (exist, _vdest_func);
                return;
        }
        object_hash_set_value_once (obj->main_table,
                                    ckey,
                                    is_vkey,
                                    value,
                                    _vdest_func,
                                    obj->table_size);
        obj->table_used += 1;
        /*判断是否需要增加表的大小*/
        if (((double)obj->table_used / (double)obj->table_size) >
            OBJ_HASH_REHASH_RATE) {
                object_hash_add_size (obj, OBJ_HASH_CHUNK_SZ);
        }
}

void
object_hash_dest_value (ObjectHash* obj, const char* ckey, bool is_vkey)
{
        o_uint        magic;
        ObjectHashKV* obj_kv;
        if (!obj || !ckey)
                return;
        obj_kv = object_hash_get_kv (obj, ckey, is_vkey);
        if (!obj_kv)
                return;
        /*如果不是头节点的话，直接调用 node 的删除方法*/
        if (OBJECT_NODE (obj_kv)->prev) {
                object_node_delete_node (OBJECT_NODE (obj_kv));
        } else {
                /*如果要删除的是头节点，则先定位 magic*/
                magic = object_hash (ckey, is_vkey, obj->table_size);
                /*更新链表头，此节点被 Hash 表所需要，故增加引用*/
                obj->main_table[magic] = OBJECT_HASH_KV (
                        object_node_get_next (OBJECT_NODE (obj_kv)));
                object_addref (obj->main_table[magic]);
                /*设置当前头为 manager*/
                object_node_set_as_manager (
                        OBJECT_NODE (obj->main_table[magic]));
                object_node_set_prev (OBJECT_NODE (obj->main_table[magic]),
                                      NULL);
                /*解引用当前节点*/
                object_unref (obj_kv);
        }
        obj->table_used -= 1;
        /*判断是否需要削减表的大小*/
        if (((double)obj->table_used / (double)obj->table_size) <
            OBJ_HASH_SHRINK_RATE) {
                object_hash_reduce_size (obj, OBJ_HASH_CHUNK_SZ);
        }
}

void
object_hash_print_table (ObjectHash* obj)
{
        ObjectHashKV* tmp;
        if (!obj)
                return;
        for (int i = 0; i < obj->table_size; ++i) {
                tmp = obj->main_table[i];
                printf ("[%d]: ", i);
                while (tmp) {
                        printf ("%s,<%s>\t",
                                object_string_get_string (tmp->ckey),
                                object_string_get_string (tmp->value));
                        tmp = OBJECT_HASH_KV (
                                object_node_get_next (OBJECT_NODE (tmp)));
                }
                printf ("\n");
        }
        printf ("已用/总共: %d / %d\n", obj->table_used, obj->table_size);
}