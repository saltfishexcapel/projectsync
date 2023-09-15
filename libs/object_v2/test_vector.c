#include <stdio.h>
#include "object_v2-vector.h"

int
main (int argc, char* argv[])
{
        ObjectVector* vec;

        vec = object_vector_new ();
        object_vector_add_datas (vec, O_PTR (52), NULL);
        object_vector_add_datas (vec, O_PTR (133), NULL);
        printf ("size: %d\n", vec->table_size);
        object_unref (vec);
        return 0;
}