/**
 * 尚不完善！！！
*/

#ifndef OBJECT_V2_VECTOR
#define OBJECT_V2_VECTOR

#include "object_v2-object.h"
#include "object_v2-type.h"

/**
 * 请注意：ObjectVectorElement 的默认减引用计数是该对象的析构，而不是消除器
*/
OBJECT_DECLARE (ObjectVectorElement, object_vector_element)
#define OBJECT_VECTOR_ELEMENT(any) ((ObjectVectorElement*)(any))
struct _ObjectVectorElement
{
        Object      parent_instance;
        o_ptr       datas;
        o_dest_func dest_func;
};

void object_vector_element_set_datas (ObjectVectorElement* obj, o_ptr datas);
void object_vector_element_set_dest_func (ObjectVectorElement* obj,
                                          o_dest_func          dest_func);

OBJECT_DECLARE (ObjectVector, object_vector)
#define OBJECT_VECTOR(any) ((ObjectVector*)(any))
struct _ObjectVector
{
        Object               parent_instance;
        int                  table_size;
        ObjectVectorElement* element_table;
        ObjectVectorElement* cache_table;
};

/**
 * @brief 添加若干 VectorElement
 * 的空位。注意，空位没有数据，就没有该空位的引用计数
 * @param obj ObjectVector 对象
 * @param seat_nums 需要增加的元素位置数量
 */
void object_vector_add_seat (ObjectVector* obj, int seat_nums);

/**
 * @brief 删除若干 VectorElement，将会调用 VectorElement 的减引用计数。
 * @param obj 
 * @param seat_nums 
 */
void object_vector_reduce_seat (ObjectVector* obj, int seat_nums);

/**
 * @brief 将 src 添加到 obj 的后面
 * @param obj ObjectVector 对象
 * @param src_vector 源数据 ObjectVector 对象
 */
void object_vector_append (ObjectVector* obj, const ObjectVector* src_vector);

/**
 * @brief 向 Vector 末尾添加一条数据
 * @param obj 
 * @param data
 * @param dest_func 
 */
void object_vector_add_datas (ObjectVector* obj, o_ptr data, o_dest_func dest_func);

/**
 * @brief 去掉 Vector 的最后一个元素
 * @param obj
 */
void object_vector_pop_end (ObjectVector* obj);
#endif