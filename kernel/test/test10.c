#include "string.h"

/*******************************************************************************
 * Test 10
 *
 * Test d'utilisation d'une file comme espace de stockage temporaire.
 ******************************************************************************/
static void
test10(void)
{
    int fid;
    char *str = "abcde";
    unsigned long len = strlen(str);
    char buf[10];

    printf("1");
    assert((fid = pcreate(5)) >= 0);
    write(fid, str, len);
    printf(" 2");
    read(fid, buf, len);
    buf[len] = 0;
    assert(strcmp(str, buf) == 0);
    assert(pdelete(fid) == 0);
    printf(" 3.\n");
}
