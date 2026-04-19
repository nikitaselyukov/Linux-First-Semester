#include <zlib.h>
#include "zlib_build_demo.hh"

const char* get_zlib_version()
{
    return zlibVersion();
}

int main()
{
    return get_zlib_version()[0] == '\0';
}
