#ifndef FILE_HASH_H
#define FILE_HASH_H

#include "../libs/object_v2/object_v2.h"

OBJECT_DECLARE (FileHash, file_hash)
#define FILE_HASH(any) ((FileHash*)(any))
struct _FileHash
{
        o_uint p1, p2, p3, p4; /*哈希的四个部分*/
};

/**
 * 生成文件哈希，出错返回 false
*/
bool file_hash_generate (FileHash* fhash, const char* file_name);

#endif