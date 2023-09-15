#include "FileHash.h"

int
main ()
{
        FileHash fhash;
        file_hash_generate (&fhash, "/bin/bash");
        
        return 0;
}