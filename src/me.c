#include <stdio.h>
#include <stdlib.h>
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

int str_prefix( const char *astr, const char *bstr )
{
 /*   if ( astr == NULL )
    {
        bug( "Strn_cmp: null astr.", 0 );
        return TRUE;
    }

    if ( bstr == NULL )
    {
        bug( "Strn_cmp: null bstr.", 0 );
        return TRUE;
    }*/
fprintf(stderr, "comparing %s to %s\n", astr, bstr);
    for ( ; *astr; astr++, bstr++ )
    {
fprintf(stderr, "comparing %c to %c\n", LOWER(*astr), LOWER(*bstr) );
        if ( LOWER(*astr) != LOWER(*bstr) )
            return 1;
    }

    return 0;
}

int main(void)
{
 char buf1[] = "The first String";
 char buf2[] = "The second String";
 fprintf(stderr, "I want to compare %s to %s\n", buf1,buf2);
 fprintf(stderr, "result is %d\n", str_prefix(buf1, buf2));
}
