#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "merc.h"

#define USE_HASH 

#define STR_HASH_SIZE	1000
#define SINGLE_STRING   65534
#define LEN_ADJ         1       /* MAX_STR_LENGTH factor - 1<64k 2<128k etc */

void		show_hash( int count );

struct hashstr_data *string_hash[STR_HASH_SIZE];
int str_leng_alloc=0;

int get_hash( char *str)
  {
  register int c1;
  register char *pt;

  for( c1=0, pt=str, str_leng_alloc=0; *pt!='\0'; str_leng_alloc++, pt++, c1++)
    c1+=*pt;

  return( c1%STR_HASH_SIZE );
  }

/*
 * Check hash table for existing occurance of string.
 * If found, increase link count, and return pointer,
 * otherwise add new string to hash table, and return pointer.
 */

  /*  Type 0=Normal 1=Single (this is not hashed ) */
char *str_alloc( char *str,int str_type )
{

#ifdef USE_HASH

   register int len, hash, psize, lnp;
   char *pt, emp[2];
   register struct hashstr_data *ptr;


   *emp='\0';
   if(str==NULL)
     str = emp;

   hash = get_hash(str);
   len = str_leng_alloc;

   lnp=len/LEN_ADJ;
   psize = sizeof(struct hashstr_data);
   if( str_type == 0 )
   for (ptr = string_hash[hash]; ptr; ptr = ptr->next )
     if ( lnp == ptr->length && !strcmp(str,(char *)ptr+psize) )
       {
       if( ptr->links < SINGLE_STRING-1 )
         {
	 ++ptr->links;
	 return (char *) ptr+psize;
         }
       }
   pt = alloc_mem(len+psize+1);
   ptr=(struct hashstr_data *)pt;

   /* Let's quickly change the header of the raw memory  -  S for String */
   set_block_type( 'S', pt );

   if( str_type == 0 )
     ptr->links		= 1;
   else
   if( str_type == 1 )
     ptr->links		= SINGLE_STRING;

   ptr->length		= lnp;
   if (len)
     strcpy( (char *) ptr+psize, str );
   else
     strcpy( (char *) ptr+psize, "" );
   ptr->next		= string_hash[hash];
   if( string_hash[hash]!=NULL )
     string_hash[hash]->prev = ptr;
   ptr->prev            = NULL;
   string_hash[hash]	= ptr;
   return (char *) ptr+psize;

#else

    static char *ret;
    int len;

   if(str==NULL )
     return(NULL);
   if( *str=='\0')
     return(str_empty);

    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
   if( str_type != 1 )
     set_block_type( 'S', pt );
   else
     set_block_type( 'P', pt );
    return ret;  
#endif
}

char *str_alloc_single( char *str)
{
  return( str_alloc( str , 1));
}

/*
 * Used to make a quick copy of a string pointer that is known to be already
 * in the hash table.  Function increments the link count and returns the
 * same pointer passed.
 */
char *quick_link( char *str )
{
#ifdef USE_HASH
    register struct hashstr_data *ptr;

   if(str==NULL)
     return( NULL );

    /* Let's hope that the CRC works here 
    if( get_string_size(str)>=0 )
      return( str_alloc( str ) ); */

    ptr = (struct hashstr_data *) (str - sizeof(struct hashstr_data));
    if ( ptr->links == 0 )
    {
	fprintf(stderr, "quick_link: bad pointer\n" );
	return NULL;
    }
    if ( ptr->links < SINGLE_STRING-1 )
	++ptr->links;
    else
        return( str_alloc( str,0) );
    return str;
#else
   if(str==NULL)
     return( NULL );
   if(*str=='\0')
     return(str_empty);
   if( *(str-2)=='P' || *(str-2)=='p' )
     return( str );
   return( str_alloc(str,0) );
#endif
}

/*
 * Used to remove a link to a string in the hash table.
 * If all existing links are removed, the string is removed from the
 * hash table and disposed of.
 * returns how many links are left, or -1 if an error occurred.
 */
int str_free( char *str )
{
#ifdef USE_HASH
    register int hash, psize;
    struct hashstr_data *ptr;
    char *pt;
    register struct hashstr_data *ptrp;

    if( str==NULL )
      return(0);

    /* Let's hope that the CRC works here 
    if( get_string_size(str)>=0 || get_alloc_size_perm(str)>=0 )
      {
      free_mem( &str, 0);
      return( 0 );
      }  */

    hash = get_hash(str);
    psize = sizeof( struct hashstr_data);
    ptr = (struct hashstr_data *) (str - psize);
    if ( ptr->links == 0 )
    {
	fprintf(stderr, "str_free: bad pointer\n" );
	return -1;
    }
    if ( ptr->links==SINGLE_STRING || --ptr->links == 0 )
    {

    ptrp = ptr->prev;
    if( ptrp!=NULL )
      {
      ptrp->next = ptr->next;
      }
    else
      {
      string_hash[hash]=ptr->next;
      }

    /* Ok let's use ptrp for the next item */
    ptrp = ptr->next;
    if( ptrp!=NULL )
      ptrp->prev = ptr->prev;


    pt = (char *)ptr;
    free_mem( &pt, 0 );
    return( 0 );

     /*
	if ( string_hash[hash] == ptr )
	{
	    string_hash[hash] = ptr->next;
            pt = (char *)ptr;
	    free_mem(&pt,0);
	    return 0;
	}
	for ( ptr2 = string_hash[hash]; ptr2; ptr2 = ptr2_next )
	{
	    ptr2_next = ptr2->next;
	    if ( ptr2_next == ptr )
	    {
		ptr2->next = ptr->next;
                pt = (char *)ptr;
		free_mem(&pt, 0);
		return 0;
	    }
	}
	fprintf( stderr, "str_free: pointer not found for string: %s\n", str );
	return -1;
     */
    }
    return ptr->links;

#else

  if( str==NULL || str==str_empty )
    return( 0 );

  free_mem( &str, 0);
  return( 0 );

#endif
}

void show_hash( int count )
{
    struct hashstr_data *ptr;
    int x, c;

    for ( x = 0; x < count; x++ )
    {
	for ( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ );
	fprintf( stderr, " %d", c );
    }
    fprintf( stderr, "\n" );
}

void hash_dump( int hash )
{
    struct hashstr_data *ptr;
    char *str;
    int c, psize;

    if ( hash > STR_HASH_SIZE || hash < 0 )
    {
	fprintf( stderr, "hash_dump: invalid hash size\n\r" );
	return;
    }
    psize = sizeof(struct hashstr_data);
    for ( c=0, ptr = string_hash[hash]; ptr; ptr = ptr->next, c++ )
    {
	str = (char *) (((int) ptr) + psize);
	fprintf( stderr, "Lnp:%4d Lnks:%5d Str: %s\n\r",
	  ptr->length, ptr->links, str );
    }
    fprintf( stderr, "Total strings in hash %d: %d\n\r", hash, c );
}

char *check_hash( char *str )
{
   static char buf[1024];
   int len, hash, psize, p = 0, c, lnp;
   struct hashstr_data *ptr, *fnd;

   buf[0] = '\0';
   len = strlen(str);
   lnp = len/LEN_ADJ;
   psize = sizeof(struct hashstr_data);
   hash = get_hash(str);
   for (fnd = NULL, ptr = string_hash[hash], c = 0; ptr; ptr = ptr->next, c++ )
     if ( lnp == ptr->length && !strcmp(str,(char *)ptr+psize) )
     {
	fnd = ptr;
	p = c+1;
     }
   if ( fnd )
     sprintf( buf, "Hash info on string: %s\n\rLinks: %d  Position: %d/%d  Hash: %d  Lnp: %d\n\r",
	  str, fnd->links, p, c, hash, fnd->length );
   else
     sprintf( buf, "%s not found.\n\r", str );
   return buf;
}

char *hash_stats( char *argument )
{
    static char buf[MAX_STRING_LENGTH];
    char buft[MAX_STRING_LENGTH];
    struct hashstr_data *ptr;
    int x, c, total, totlinks, unique, bytesused, wouldhave, hilink;
    int nonhashed, avgtotal, devtotal, hihash, hihashx;
    int hashamounts[STR_HASH_SIZE];

    totlinks = unique = total = bytesused = wouldhave = hilink = 0;
    nonhashed=0;
    devtotal=0;
    hihashx=0;
    hihash=0;
    *buf='\0';
    for ( x = 0; x < STR_HASH_SIZE; x++ )
    {
        hashamounts[x]=0;
	for ( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ )
	{
	   total++;
           hashamounts[x]++;
	   if ( ptr->links == 1 )
	     unique++;
	   if ( ptr->links > hilink  && ptr->links< SINGLE_STRING )
	     hilink = ptr->links;
	   if ( ptr->links<SINGLE_STRING )
	     totlinks += ptr->links;
           else
             nonhashed++;
	   bytesused += (ptr->length*LEN_ADJ + 1 + sizeof(struct hashstr_data));
	   if ( ptr->links<SINGLE_STRING )
	     wouldhave += (ptr->links * (ptr->length*LEN_ADJ + 1));
        }
        if( !strcasecmp( argument, "links" ) )
          {
          sprintf( buft, "  %5d:%-6d      ", x, hashamounts[x] );
          strcat( buf, buft );
          }
    }
    avgtotal = total/STR_HASH_SIZE;
    for ( x = 0; x < STR_HASH_SIZE; x++ )
      {
      devtotal += abs( hashamounts[x]-avgtotal );
      if( hashamounts[x] > hihash )
        {
        hihash = hashamounts[x];
        hihashx = x;
        }
      }
    devtotal /= STR_HASH_SIZE;

    sprintf( buft, "Hash strings allocated:%8d  Total links  : %d\n\rString bytes allocated:%8d  Bytes saved  : %d\n\rUnique (wasted) links :%8d  Hi-Link count: %d\n\rNonhashed Strings     : %d\n\rAverage Hashs         : %-8d  StdDev Hashs: %-8d\n\rMaximum Hashs         : %-8d  (Hash=%d)\n\r",
	total, totlinks, bytesused, wouldhave - bytesused, unique, hilink,
        nonhashed, avgtotal, devtotal, hihash, hihashx );
    strcat( buf, buft );
    return buf;
}

void show_high_hash( int top )
{
    struct hashstr_data *ptr;
    int x, psize;
    char *str;

    psize = sizeof(struct hashstr_data);
    for ( x = 0; x < STR_HASH_SIZE; x++ )
	for ( ptr = string_hash[x]; ptr; ptr = ptr->next )
	  if ( ptr->links >= top )
	  {
	     str = (char *) (((int) ptr) + psize);
 	     fprintf( stderr, "Links: %5d  String: >%s<\n\r", ptr->links, str );
	  }
}

