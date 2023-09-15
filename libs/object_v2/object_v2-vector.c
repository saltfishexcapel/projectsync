#include "object_v2-vector.h"

void
object_vector_element_init (ObjectVectorElement* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, object_vector_element_destruction);
        obj->datas     = NULL;
        obj->dest_func = NULL;
        return;
}

ObjectVectorElement*
object_vector_element_new ()
{
        ObjectVectorElement* obj;
        obj = OBJECT_NEW (ObjectVectorElement);
        object_vector_element_init (obj);
        return obj;
}

void
object_vector_element_destruction (ObjectVectorElement* obj)
{
        if (!obj)
                return;
        if (obj->dest_func)
                obj->dest_func (obj->datas);
        return;
}

void
object_vector_element_destory (ObjectVectorElement* obj)
{
        if (!obj)
                return;
        object_vector_element_destruction (obj);
        free (obj);
}

void
object_vector_element_set_datas (ObjectVectorElement* obj, o_ptr datas)
{
        if (!obj)
                return;
        if (obj->dest_func && obj->datas)
                obj->dest_func (obj->datas);
        obj->datas = datas;
}

void
object_vector_element_set_dest_func (ObjectVectorElement* obj,
                                     o_dest_func          dest_func)
{
        if (obj)
                obj->dest_func = dest_func;
}

void
object_vector_init (ObjectVector* obj)
{
        if (!obj)
                return;
        object_init (OBJECT (obj));
        object_set_destory_func (obj, object_vector_destory);
        obj->cache_table   = NULL;
        obj->element_table = NULL;
        obj->table_size    = 0;
}

ObjectVector*
object_vector_new ()
{
        ObjectVector* obj;
        obj = OBJECT_NEW (ObjectVector);
        object_vector_init (obj);
        return obj;
}

void
object_vector_destruction (ObjectVector* obj)
{
        if (!obj)
                return;
        if (obj->table_size == 0)
                return;
        for (int i = 0; i < obj->table_size; ++i)
                object_unref (&obj->element_table[i]);
        free (obj->element_table);
}

void
object_vector_destory (ObjectVector* obj)
{
        if (!obj)
                return;
        object_vector_destruction (obj);
        free (obj);
        return;
}

/**
 * @brief 添加若干 VectorElement
 * 的空位。注意，空位没有数据，就没有该空位的引用计数
 * @param obj ObjectVector 对象
 * @param seat_nums 需要增加的元素位置数量
 */
void
object_vector_add_seat (ObjectVector* obj, int seat_nums)
{
        if (!obj || seat_nums <= 0)
                return;
        obj->table_size += seat_nums;
        if (obj->element_table) {
                obj->element_table = OBJECT_VECTOR_ELEMENT (
                        realloc (obj->element_table,
                                 (obj->table_size) * sizeof (ObjectVector)));
        } else {
                obj->element_table = OBJECT_VECTOR_ELEMENT (
                        malloc ((obj->table_size) * sizeof (ObjectVector)));
        }
}

void
object_vector_reduce_seat (ObjectVector* obj, int seat_nums)
{
        if (!obj || seat_nums <= 0)
                return;
        // 限制 seat_nums 的最大大小
        if (seat_nums > obj->table_size)
                seat_nums = obj->table_size;
        // 将需要丢掉的部分解引用
        for (int i = 0; i < seat_nums; ++i)
                object_unref (&obj->element_table[obj->table_size - 1 - i]);
        if (obj->table_size - seat_nums)
                obj->element_table = OBJECT_VECTOR_ELEMENT (
                        realloc (obj->element_table,
                                 (obj->table_size - seat_nums)));
        else if (obj->table_size) {
                free (obj->element_table);
                obj->element_table = NULL;
        }
        obj->table_size -= seat_nums;
}

static void
object_vector_copy_tool1 (ObjectVectorElement* start_copy,
                          const ObjectVector*  src_vector)
{
        // memcpy ()
        for (int i = 0; i < src_vector->table_size; ++i) {
                start_copy[i] = src_vector->element_table[i];
                object_addref (&start_copy[i]);
        }
}

#ifndef MIN_TABLE_SZ
        #define MIN_TABLE_SZ 0
void
object_vector_append (ObjectVector* obj, const ObjectVector* src)
{
        int old_size;
        if (!obj || !src)
                return;
        if (src->table_size <= MIN_TABLE_SZ)
                return;
        old_size = obj->table_size;
        object_vector_add_seat (obj, src->table_size);
        // src的数据从 old_size 开始拷贝
        object_vector_copy_tool1 (&obj->element_table[old_size], src);
}

void
object_vector_add_datas (ObjectVector* obj, o_ptr data, o_dest_func dest_func)
{
        ObjectVectorElement element;
        object_vector_element_set_datas (&element, data);
        object_vector_element_set_dest_func (&element, dest_func);
        // object_vector_append (obj, &element);
        object_vector_add_seat (obj, 1);
        obj->element_table[obj->table_size - 1] = element;
        object_addref (&element);
}
        #undef MIN_TABLE_SZ
#endif

void
object_vector_pop_end (ObjectVector* obj)
{
        object_vector_reduce_seat (obj, 1);
}