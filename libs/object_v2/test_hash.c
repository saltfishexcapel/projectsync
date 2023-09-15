#include "object_v2-hash.h"
#include <stdio.h>

int
main ()
{
        ObjectHash* hash;
        o_ptr pointer;

        hash = object_hash_new ();
        object_hash_set_value (hash, "abc", false, O_PTR (5667), NULL);

        pointer = object_hash_get_value (hash, "abc", false);

        printf ("pointer: %p\n", pointer);

        object_unref (hash);
        return 0;
}