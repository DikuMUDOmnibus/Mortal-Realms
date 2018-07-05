/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file first_content all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <stdarg.h>

#include "merc.h"

const char sector_name[8][20] = 
  { "Inside", "City", "Field", "Forest", "Hills", "Mountains", "Shallow Water", "Deep Water" };
/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

void scan_for_duplication( CHAR_DATA *);
char prompt_buffer[MAX_INPUT_LENGTH];
char *prompt_return( CHAR_DATA *, char *);

static void StupidAdmin();
#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif

/*int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );*/

#define INSTRUCTION_PAGES 20

int comm_status;
int pager( DESCRIPTOR_DATA *, const char *, int, char *);
bool scroll( DESCRIPTOR_DATA *, const char *, bool, int);
char *ansi_strip( char *);
char  ansi_strip_txt[MAX_STRING_LENGTH];
bool scroll_you( DESCRIPTOR_DATA *, const char *, bool);
bool PAGER_PAUSE;
void scan_objects_container( CHAR_DATA *);
void remove_bad_desc( CHAR_DATA *, bool);
void remove_bad_desc_name( char *);
bool find_ch_game( CHAR_DATA *);
bool find_ch_room( CHAR_DATA *);
bool find_ch_player( CHAR_DATA *);
CHAR_DATA *scan_char( DESCRIPTOR_DATA *, char * );
int count_links( HOST_NAMES *);

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
char	echo_off_str	[] = { '\0' };
char	echo_on_str	[] = { '\0' };
char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
char 	go_ahead_str	[] = { IAC, GA, '\0' };

#include <sys/wait.h>
/* int wait4( void ); */
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	getsockopt	args( ( int s, int level, int optname, void *optval,
			    int *optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	getsockopt	args( ( int s, int level, int optname, void *optval,
			    int *optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if	defined(linux)
/* int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );*/
/*int	bind		args( ( int s, struct sockaddr *name, int namelen ) );*/
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
/*int	read		args( ( int fd, char *buf, int nbyte ) );*/
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
/*int	write		args( ( int fd, char *buf, int nbyte ) );*/
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	getsockopt	args( ( int s, int level, int optname, void *optval,
			    int *optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * This includes Solaris SYSV as well.
 */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, const struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
/*intgettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );*/
int	listen		args( ( int s, int backlog ) );
ssize_t	read		args( ( int fd, void *buf, size_t nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int	setsockopt	args( ( int s, int level, int optname, 
                            const char *optval, int optlen ) );
int	getsockopt	args( ( int s, int level, int optname, char *optval,
			    int *optlen ) );
#else 
#endif
int	socket		args( ( int domain, int type, int protocol ) );
ssize_t	write		args( ( int fd, const void *buf, size_t nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	getsockopt	args( ( int s, int level, int optname, void *optval,
			    int *optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   first_descriptor;	/* All open descriptors		*/
DESCRIPTOR_DATA *   last_descriptor;	/* last open descriptors	*/

DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* Time of this pulse		*/
time_t		    rent_time;	/* Time of this pulse		*/
int		    num_descriptors;
bool                more_than_one_close;
char                lastplayercmd[MAX_INPUT_LENGTH*2];
CHAR_DATA * Greeter = NULL;
CHAR_DATA * Greeted = NULL;

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control, int port ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control,  int port) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_reconnecting		args( ( DESCRIPTOR_DATA *d,CHAR_DATA *ch)); 
int	main			args( ( int argc, char **argv ) );
bool	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	creator			( DESCRIPTOR_DATA *, char *);
void	display_creator		( DESCRIPTOR_DATA *);
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
int nice( int );
/* void  check_player_hosts( char *); */

#define TEST_GAME_PORT 9999
int  port,control;

int main( int argc, char **argv )
{
    struct timeval now_time;
    struct tm now;

    bool fCopyOver  = FALSE;
    int cnt;
    num_descriptors = 0;
    total_memory_warnings = 0;
    first_descriptor = NULL;
    last_descriptor  = NULL;
    first_bounty     = NULL;
    last_bounty      = NULL;
    str_empty = str_dup( "x" );
    *str_empty = '\0';

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif


 /* Let's log the current system constraints as seen by MrMud - Chaos 5/3/98 */
    {
    char bufm[200];
    struct rlimit rlpt;

    getrlimit( RLIMIT_DATA, &rlpt );
    sprintf( bufm, "System memory usage: %ld max %ld current", 
	(long)rlpt.rlim_max, (long)rlpt.rlim_cur);
    log_string( bufm );
    rlpt.rlim_max= RLIM_INFINITY;
    rlpt.rlim_cur= RLIM_INFINITY;
    setrlimit( RLIMIT_DATA, &rlpt );
    getrlimit( RLIMIT_DATA, &rlpt );
    sprintf( bufm, "Unlimited memory usage: %ld max %ld current", 
	(long)rlpt.rlim_max, (long)rlpt.rlim_cur);
    log_string( bufm );

    }

    /* test memory allocations   
  for( cnt2=0; cnt2<4; cnt2++ )
    {
    char *pt, *pto;
    int cnt;
    char bufm[100];

    pto = STRALLOC( "Testing." );
    for( pt=pto, cnt=0; cnt<strlen(pto); cnt++)
      {
      sprintf( bufm, "Testing byte %2d - %c", cnt , *pt);
      log_string( bufm );
      pt++;
      }
    sprintf( bufm, "Block size %d", get_string_size( (unsigned char*)pto ) );
    log_string( bufm );
    STRFREE (pto );
    }  

  log_string( "Testing over." );
  abort(); */

    /*
     * Init time.
     */
    comm_status=0;
    FINGER_MODE=FALSE;
    ALLOW_OUTPUT = TRUE;
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    now = *localtime(&current_time);
   while(now.tm_wday!=0)
   {
    if (++now.tm_wday>6) now.tm_wday=0;
    now.tm_yday++;
    if ( (isleap(now.tm_year+1900) && now.tm_mon==1 && ++now.tm_mday>29)||
        ( ++now.tm_mday>monthdays[now.tm_mon] ) )
    {
      now.tm_mon++;
      now.tm_mday=0;
    }
   };
   
    now.tm_min=59;
    now.tm_hour=23;
    now.tm_sec=59;

    rent_time = mktime(&now);   
    strcpy( str_boot_time, ctime( &current_time ) );

    /* Allocate memory for the continuous tactical map */
    CREATE(cont_tact, TACTICAL_MAP, 1);

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Get the port number.
     */
    port = 4321;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    abort( );
	}
	else if ( ( port = atol( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    abort( );
	}
        nice( 20 );
        REAL_GAME = FALSE;
    }
    else
        {
            /*   Use this only on dedicated machines  */
        nice( -10 ); 
            /*   Use this only on non-dedicated machines 
                 nice( 39 ); */
        REAL_GAME = TRUE;
        }

   if (port == 4321 ) 
     {
     REAL_GAME = TRUE;
     TEST_GAME = FALSE;
     }
   else
     {
     REAL_GAME = FALSE;
     TEST_GAME = TRUE;
     }


  if (!strcmp(argv[0], "Mortal")) 
       {
               fCopyOver = TRUE;
               control = atoi(argv[3]);
       }
       else
               fCopyOver = FALSE;
    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "MrMud is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
    load_sites();   /* Order 12/1/93  */
    load_bounties();   /* Presto 2-20-99 */
    for (cnt=0;cnt<VICTORY_LIST_SIZE;cnt++)
	victory_list[cnt]= STRALLOC("");
    load_victors();
    boot_db( fCopyOver );

    clear_castles();

    if (!fCopyOver)
    {
      control = init_socket( port );
    }
    sprintf( log_buf, "MrMud is ready on port %d.", port );
    log_string( log_buf );
    fpReserve = fopen( NULL_FILE, "r" );
    fpAppend = fopen( NULL_FILE, "r" );

    mix_race_war();
    dual_flip = FALSE;
    last_dead = NULL;
    DoingLookup=FALSE;
    for (cnt=0;cnt<MOST_MOST;cnt++)
     Most_Values[cnt]=0;
    game_loop_unix( control, port );
    close( control );
    save_all_clans();
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}
/*
static void SegVio()
{
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );

 for ( d = first_descriptor; d; d = d->next )
  {
    sprintf( buf, "PC: %-20s room: %d", d->character->name,
        d->character->in_room->vnum );
    log_string( buf );
  }

  abort();
}
*/
static void StupidAdmin()
{
  fprintf(stderr, "Recieved SIGUSR1. Trying graceful shutdown.\n");
  exit(-1);
} 
void ListCheck(void)
{
 CHAR_DATA *tmp;
 int count=0;

 CHECK_LINKS(first_char, last_char, next, prev, CHAR_DATA);
 for(tmp=first_char;tmp!=NULL;tmp=tmp->next)
  count++;
 log_printf( "There are currently %d characters in the first_char list", count); 
}
#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x=1;
    int fd;
    char buf[200];

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	abort( );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	abort( );
    }

    {
	struct	linger	ld;

	ld.l_onoff  = 0;
	ld.l_linger = 100;

	if ( setsockopt( fd, SOL_SOCKET, SO_LINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_LINGER" );
	    close( fd );
	    abort( );
	}
    }

    {
    int sockbuf;
    int socksize;

    socksize = sizeof( sockbuf);
    if ( getsockopt( fd, SOL_SOCKET, SO_SNDBUF,
        (char *) &sockbuf, (int *) &socksize ) < 0 )
      {
      perror( "getsockopt: SO_SNDBUF" );
      close( fd );
      abort( );
      }
    if(comm_status==0)
      {
    sprintf( buf, "Initial output buffer size: %d", sockbuf);
    log_string( buf );
      }

    sockbuf = 2048;
    if ( setsockopt( fd, SOL_SOCKET, SO_RCVBUF,
        (char *) &sockbuf, sizeof(socksize) ) < 0 )
      {
      perror( "getsockopt: SO_RCVBUF" );
      close( fd );
      abort( );
      }
    if(comm_status==0)
      {
    sprintf( buf, "Input buffer size: %d", sockbuf);
    log_string( buf );
      }

    while( setsockopt( fd, SOL_SOCKET, SO_SNDBUF,
        (char *) &sockbuf, sizeof(sockbuf) ) >= 0 && sockbuf< MAX_STRING_LENGTH/2 )
         sockbuf+=1000;
    if( setsockopt( fd, SOL_SOCKET, SO_SNDBUF,
        (char *) &sockbuf, sizeof(sockbuf) ) < 0 )
      {
      perror( "getsockopt: SO_SNDBUF" );
      close( fd );
      abort( );
      }

    if ( getsockopt( fd, SOL_SOCKET, SO_SNDBUF,
        (char *) &sockbuf, (int *) &socksize ) < 0 )
      {
      perror( "getsockopt: SO_SNDBUF" );
      close( fd );
      abort( );
      }
    if(comm_status==0)
      {
    sprintf( buf, "Modified output buffer size: %d", sockbuf);
    log_string( buf );
      }
    comm_status++;
    }

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror( "Init_socket: bind" );
	close( fd );
	abort( );
    }

    if ( listen( fd, 20 ) < 0 )
    {
	perror( "Init_socket: listen" );
	close( fd );
	abort( );
    }

    return fd;
}
#endif


#if defined(unix)
void game_loop_unix( int control , int port)
{
    static struct timeval null_time;
    struct timeval last_time;
    DESCRIPTOR_DATA *d, *d_next;
    int leng, dcnt;
    char dbuf[180];


    /* These are very important. Do not comment out for any reason
	Martin 10/3/99 */
    signal( SIGUSR1, StupidAdmin );
    signal( SIGPIPE, SIG_IGN );
    /*signal( SIGTERM, StupidAdmin );
    signal( SIGKILL, StupidAdmin );*/
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;
    merc_reboot=FALSE;
    d_next=NULL;

    more_than_one_close=FALSE;

    ALLOW_OUTPUT = TRUE;

    /* Main loop */
    while ( !merc_down  )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	int maxdesc;
        if( first_descriptor==NULL && merc_reboot)
          merc_down=TRUE;

        equipment_affecting = FALSE;

        open_timer( TIMER_SCAN_DESC );


#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

       /* Terminate any children at rate of 4/sec 
       wait4(); */

	/*
	 * Poll all active descriptors.
	 */

      for( dcnt=0; dcnt<255; dcnt++)
        if ( getsockopt( dcnt, SOL_SOCKET, SO_ERROR,
        (char *) &leng, (int *) &leng ) >= 0 )
          if( leng != 0 )
           perror( "scansocket: SO_ERROR" );


	if (Greeter !=NULL && Greeted !=NULL && !IS_AFFECTED(Greeter, AFF2_POSSESS) && !IS_AFFECTED(Greeted, AFF2_POSSESS))
 	  mprog_percent_check( Greeter, Greeted, NULL, NULL,GROUP_GREET_PROG );

	Greeter = NULL;
	Greeted = NULL;


        tme = *localtime(&current_time);

	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = first_descriptor; d; d = d->next )
	{
         /* remove switched bit if possible */
          if( d->original == NULL && d->character != NULL &&
              !IS_NPC( d->character ) &&
              d->character->desc == d && d->character->pcdata != NULL )
            d->character->pcdata->switched = FALSE;

	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    abort( );
	}



	/*
	 * New connection?
	 */
	  if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control, port);



	/*
	 * Kick out the freaky folks.
	 */
	for ( d = first_descriptor; d != NULL; d = d_next )
	{
	    d_next = d->next;   
      
	    if ( FD_ISSET( d->descriptor, &exc_set ) || 
                  IS_SET( d->comm_flags, COMM_FLAG_DISCONNECT))
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character 
		&&( d->connected == CON_PLAYING 
		||  d->connected == CON_EDITING ) )
		    save_char_obj(d->character, NORMAL_SAVE);
                *d->outbuf      = '\0';
		d->outtop	= 0;
	/*	log_printf("kicking off %s for being freaky\n",d->character->name);*/
		close_socket( d , TRUE);
	    }
	}

  close_timer( TIMER_SCAN_DESC );

#ifdef undef
      /* Get rid of bad items  -  Chaos  3/2/97  */
  {
  OBJ_DATA *obj, *obj_next, *objc, *objc_next;

  for( obj=first_object; obj!=NULL; obj=obj_next )
    {
    obj_next = obj->next;

    if( IS_SET( obj->extra_flags, ITEM_NOT_VALID ) )
      {

          /* Let's dump out this object */
	if (obj->first_content != NULL)
	  for (objc = obj->first_content; objc != NULL; objc = objc_next)
	    {
	      objc_next = objc->next_content;
	      obj_from_obj (objc);
	      if (obj->carried_by == NULL)
		{
		  if (obj->in_room != NULL)
		    {
		      obj_to_room (objc, obj->in_room);
		      objc->sac_timer = OBJ_SAC_TIME;
		    }
		  else
		    extract_obj (objc);
		}
	      else
		obj_to_room (objc, obj->carried_by->in_room);
	    }

      if( obj->carried_by!=NULL && obj->wear_loc!=WEAR_NONE )
        remove_obj(obj->carried_by,obj->wear_loc,TRUE, FALSE);

        extract_obj( obj );
      }
    }
  }
#endif

	/*
	 * Process input.
	 */
  if( first_descriptor!=NULL && !IS_BOOTING)
      for ( d = first_descriptor; d != NULL; d = d_next )
	{
        d_next	= d->next;
        d->fcommand	= FALSE;

        open_timer( TIMER_READ_DESC );
	if ( FD_ISSET( d->descriptor, &in_set ) )
        if( d->character == NULL || d->character->desc != d ||
          ( d->character->obj_prog_ip == 0 && d->character->alias_ip == 0 ) )
	  {
	  if ( !read_from_descriptor( d ) )
	    {
	    FD_CLR( d->descriptor, &out_set );
	    if ( d->character != NULL 
                && ( d->connected == CON_PLAYING
		||   d->connected == CON_EDITING ) )
	      save_char_obj(d->character, NORMAL_SAVE);
                *d->outbuf      = '\0';
	    d->outtop	= 0;
	    close_socket( d , TRUE);
            close_timer( TIMER_READ_DESC );
	    continue;
	    }
	  }

	if ( d->character != NULL && d->character->wait > 0  && 
             !IS_NPC(d->character))
	  {
          bool HighSpeed;
          HighSpeed=FALSE;
	  if ( d->incomm[0] == '\0' )
            if( (d->connected== CON_PLAYING || d->connected == CON_EDITING)
                 && d->character!=NULL && 
                !IS_NPC( d->character) && d->character->position==POS_FIGHTING)
              if( d->character->pcdata->auto_flags==AUTO_QUICK)
                HighSpeed=TRUE;
          d->character->pcdata->old_wait = d->character->wait;
          if( HighSpeed && d->character->hit > d->character->wimpy)
            {
	    d->character->wait-=2;
            if( d->character->wait < 0)
              d->character->wait=0;
            }
          else
            {
	     --d->character->wait;
            if( d->character->wait < 0)
              d->character->wait=0;
            }
          if(d->character->wait==0 && d->character->pcdata->old_wait!=0) 
            if(d->character->vt100==1 && IS_SET( d->character->act, PLR_PROMPT))
              {
              vt100prompt( d->character );
              }
           continue;
	  }

              /* Reset those characters from obj_prog stuff */
	   if (  *d->incomm == '\0' &&
                (d->connected== CON_PLAYING 
		||  d->connected == CON_EDITING ) &&
                d->character!=NULL && 
                d->original == NULL &&
                d == d->character->desc &&
                *d->inbuf == '\0' && 
                d->intop == 0 )
             {
             d->character->trust = d->character->level; 
	     d->character->alias_ip = 0;
             if( d->back_buf != NULL )
               {
               d->intop=str_cpy_max(d->inbuf,d->back_buf,MAX_INPUT_LENGTH-10);
               STRFREE(d->back_buf );
               d->back_buf = NULL;
               if( *d->inbuf == '\r' && d->intop==1 )
                 {
                 *d->inbuf = '\0';
                 d->intop = 0;
                 }
	       d->character->trust = d->character->level; 
               d->character->obj_prog_ip=0;
	       d->character->alias_ip=0;
               }
             }
             read_from_buffer( d );

              /* Auto reactions */
	  if (  d->incomm[0] == '\0' &&
                (d->connected== CON_PLAYING ||
                 d->connected== CON_EDITING ) &&
                d->character!=NULL && 
                d->original == NULL &&
                d == d->character->desc &&
                d->character->in_room != NULL &&
                /*d->character->position==POS_FIGHTING &&*/
                d->character->fighting != NULL &&
                d->character->pcdata->auto_flags!=AUTO_OFF &&
                d->character->hit > d->character->wimpy &&
                d->character->wait == 0 )
                {
                int sn;
		bool hadtostand = FALSE;
		switch(d->character->position)
    	        {
                  case POS_SLEEPING:
                  case POS_RESTING:
                    do_stand(d->character,"");
                    hadtostand = TRUE;
                    break;
                  case POS_STANDING:
                    d->character->position=POS_FIGHTING;
                    break;
                  case POS_FIGHTING:
                    break;
                  default:
                    return;
                    break;
                  }
		if (hadtostand) break;
                sn=skill_lookup( d->character->pcdata->auto_command );
                if( sn!=-1)
                  {
                  if( is_spell( sn))
                    {
                    if(d->character->mana >= 3 * get_mana( d->character, sn))
                      {
                      d->character->wait = 2;
                      sprintf( dbuf, "%ccast '%s'", 27, skill_table[sn].name);
                      str_cpy_max( d->incomm, dbuf, MAX_INPUT_LENGTH);
                      }
                    }
                  else
                    if(d->character->move >= 3 * skill_table[sn].min_mana &&
                       sn!=gsn_backstab)
                      {
                      d->character->wait = 2;
                      sprintf( dbuf, "%c%s", 27, skill_table[sn].name);
                      str_cpy_max( d->incomm, dbuf, MAX_INPUT_LENGTH);
                      }
                  }
               }
#ifdef undef
          if (  d->incomm[0] == '\0' &&
               (d->connected== CON_PLAYING ||
                d->connected== CON_EDITING) &&
                d->character!=NULL && 
                d->original == NULL &&
                d == d->character->desc &&
                d->character->wait == 0 &&
	        !IS_NPC(d->character) &&
                d->character->pcdata->tracking[0] != '\0' &&
                d->character->in_room != get_room_index( ROOM_VNUM_LIMBO )
              )
    {

    int cnt = 0;
    bool found = FALSE, inroom = FALSE;
    CHAR_DATA * rch;
    for(rch=d->character->in_room->first_person;rch !=NULL; rch=rch->next_in_room)
    {
     if (!strcasecmp(rch->name, d->character->pcdata->tracking))
      {
       inroom=TRUE;
       if (IS_AFFECTED(d->character, AFF_HUNT) && !IS_NPC(d->character)
           && number_percent() < d->character->pcdata->learned[gsn_greater_hunt])
       {
        send_to_char("You've found your prey!\n\r", d->character);
        do_murder(d->character, d->character->pcdata->tracking);
        break;
       }
      }
    }
    if (!inroom)
    for(cnt=0;cnt<MAX_LAST_LEFT;cnt++)
     if (!strcasecmp(d->character->pcdata->tracking,d->character->in_room->last_left[cnt]))
     {
      ch_printf(d->character, "You find the tracks of %s, and follow them.\n\r",
        capitalize(d->character->pcdata->tracking));
      found = TRUE;
      break;
     }
     if (found)
     {
         if (IS_SET(d->character->in_room->last_left_bits[cnt], TRACK_NORTH))
         {
           if (d->character->in_room->exit[DIR_NORTH]->to_room 
		== d->character->was_in_room )
            {
 	      send_to_char ("Your prey has doubled back!\n\r",d->character);    
              STRFREE (d->character->pcdata->tracking );
              d->character->pcdata->tracking = STRALLOC("");
  	    }
           else
            do_north(d->character, "");
         }
         else if (IS_SET(d->character->in_room->last_left_bits[cnt], TRACK_SOUTH))
 	 {
           if (d->character->in_room->exit[DIR_SOUTH]->to_room 
		== d->character->was_in_room )
            {
 	      send_to_char ("Your prey has doubled back!\n\r",d->character);    
              STRFREE (d->character->pcdata->tracking );
              d->character->pcdata->tracking = STRALLOC("");
  	    }
           else
            do_south(d->character, "");
         }
         else if (IS_SET(d->character->in_room->last_left_bits[cnt], TRACK_EAST))
         {
           if (d->character->in_room->exit[DIR_EAST]->to_room 
		== d->character->was_in_room )
            {
 	      send_to_char ("Your prey has doubled back!\n\r",d->character);    
              STRFREE (d->character->pcdata->tracking );
              d->character->pcdata->tracking = STRALLOC("");
  	    }
           else
            do_east(d->character, "");
         }
         else if (IS_SET(d->character->in_room->last_left_bits[cnt], TRACK_WEST))
         {
           if (d->character->in_room->exit[DIR_WEST]->to_room 
		== d->character->was_in_room )
            {
 	      send_to_char ("Your prey has doubled back!\n\r",d->character);    
              STRFREE (d->character->pcdata->tracking );
              d->character->pcdata->tracking = STRALLOC("");
  	    }
           else
            do_west(d->character, "");
         }
         else if (IS_SET(d->character->in_room->last_left_bits[cnt], TRACK_UP))
         {
           if (d->character->in_room->exit[DIR_UP]->to_room 
		== d->character->was_in_room )
            {
 	      send_to_char ("Your prey has doubled back!\n\r",d->character);    
              STRFREE (d->character->pcdata->tracking );
              d->character->pcdata->tracking = STRALLOC("");
  	    }
           else
            do_up(d->character, "");
         }
         else if (IS_SET(d->character->in_room->last_left_bits[cnt], TRACK_DOWN))
         {
           if (d->character->in_room->exit[DIR_DOWN]->to_room 
		== d->character->was_in_room )
            {
 	      send_to_char ("Your prey has doubled back!\n\r",d->character);    
              STRFREE (d->character->pcdata->tracking );
              d->character->pcdata->tracking = STRALLOC("");
  	    }
           else
            do_down(d->character, "");
         }
     }
     else
     {
      ch_printf(d->character, "%s's tracks end here.\n\r", capitalize(d->character->pcdata->tracking));
      STRFREE (d->character->pcdata->tracking );
      d->character->pcdata->tracking = STRALLOC("");
     }
    }
#endif
              /* Travel Commands */
	  if (  d->incomm[0] == '\0' &&
               (d->connected== CON_PLAYING ||
                d->connected== CON_EDITING) &&
                d->character!=NULL && 
                d->original == NULL &&
                d == d->character->desc &&
                d->character->wait == 0 &&
                d->character->in_room != get_room_index( ROOM_VNUM_LIMBO ) &&
                d->character->pcdata->travel >= 0 )
            {
            if( d->character->in_room != NULL &&
                d->character->position==POS_STANDING &&
                d->character->fighting == NULL &&
                d->character->hit > d->character->wimpy )
                {
                int exit_cnt, new_exit, cnt;
                ROOM_INDEX_DATA *in_room;
                CHAR_DATA *ch;

                ALLOW_OUTPUT = FALSE;
                ch = d->character;
                exit_cnt=0;
                new_exit=0;
                cnt=0;
                in_room = ch->in_room;
                for(; cnt<6; cnt++)
                  if( cnt!=reverse(d->character->pcdata->travel) &&
                      ch->in_room->exit[cnt] != NULL )
                    {
                    if( can_move_char( ch, cnt )
   && ( !IS_SET(ch->in_room->exit[cnt]->to_room->room_flags,ROOM_IS_CASTLE)  
	                || IS_NPC( ch ) || ch->pcdata==NULL
	    || !IS_SET(ch->pcdata->player2_bits, PLR2_CASTLES)) )
                      {
                      if( ch->pcdata->travel_from == NULL ||
                ch->pcdata->travel_from != ch->in_room->exit[cnt]->to_room )
                        {
                        new_exit = cnt;
                        exit_cnt ++;
                        }
                      }
                    }
                  ALLOW_OUTPUT = TRUE;
                  if( exit_cnt != 1 )
                    {
                    if( exit_cnt < 1 )
                      {
                      if( ch->move > 15 )
                      send_to_char( "You stop traveling due to a dead end.\n\r", ch );
                      else
                      send_to_char( "You stop traveling to rest.\n\r", ch );
                      }
                    else
                      send_to_char( "You stop traveling to pick directions.\n\r", ch );
                    ch->pcdata->travel = -1;
                    ch->pcdata->travel_from = NULL;
                    }
                  else
                    {
                    ch->pcdata->travel = new_exit;
                    ch->pcdata->travel_from = ch->in_room;
                    switch( new_exit )
                      {
                 case 0: send_to_char( "You travel north.\n\r", ch ); break;
                 case 1: send_to_char( "You travel east.\n\r", ch ); break;
                 case 2: send_to_char( "You travel south.\n\r", ch ); break;
                 case 3: send_to_char( "You travel west.\n\r", ch ); break;
                 case 4: send_to_char( "You travel up.\n\r", ch ); break;
                 case 5: send_to_char( "You travel down.\n\r", ch ); break;
                      }
                    move_char( ch, new_exit );
                    }

                  }
              else
                {
                d->character->pcdata->travel = -1;
                d->character->pcdata->travel_from = NULL;
                send_to_char( "You stop traveling.\n\r", d->character );
                }
              }


        close_timer( TIMER_READ_DESC );



	  if ( d->incomm[0] != '\0' )
	    {
            DESCRIPTOR_DATA *dn;
	    d->fcommand	= TRUE;

            if( d->character!=NULL && 
              (d->connected == CON_PLAYING || d->connected == CON_EDITING))
              {
	      stop_idling( d->character );
              d->character->timer = 0;
              }
           for ( dn = first_descriptor; dn != NULL; dn = dn->next )
             if( dn!=d && dn->character == d->character && 
                 dn->character != NULL && (dn->connected!=CON_PLAYING
					 && dn->connected!=CON_EDITING))
  		{
		/*fprintf(stderr, "we dont like you\n");*/
                 SET_BIT( dn->comm_flags, COMM_FLAG_DISCONNECT);
 		}
          if( !IS_SET( d->comm_flags, COMM_FLAG_DISCONNECT ) )
            {
	    if ( d->connected == CON_PLAYING )
              {
	      interpret( d->character, d->incomm );
              }
	    else if ( d->connected == CON_EDITING )
              {
	      edit_buffer ( d->character, d->incomm );
              }
	    else
              {
              more_than_one_close=FALSE;
	      if( nanny( d, d->incomm ))
                {
                continue;
                }
              if( more_than_one_close)
                {
                d=NULL;
                continue;
                }
              }
            }
              *d->incomm = '\0';
        if( !IS_SET( d->comm_flags, COMM_FLAG_DISCONNECT ) )
	  if ( d->connected == CON_PLAYING  || d->connected == CON_EDITING)
            if( d->character!=NULL && !IS_NPC( d->character))
              if( d->character->vt100==1 && 
                        IS_SET(d->character->act, PLR_PROMPT))
                write_to_buffer( d, "\033[0K", 1000000);

	  }
        }
     
  if( more_than_one_close)
    {
    more_than_one_close=FALSE;
    log_string( "More than one close.");
    continue;
    }
	/*
	 * Autonomous game motion.
	 */
	update_handler( );
	/*
	 * Output.
	 */
  open_timer( TIMER_WRITE_DESC );
  if( first_descriptor!=NULL && !IS_BOOTING)
	for ( d = first_descriptor; d != NULL; d = d_next )
	  {
	  d_next = d->next;

              /* Combat code time based update - Chaos 2/23/96 */
          if( d->combat_buffer_length > 0 )
            {
            leng = d->combat_buffer_length;
            write_to_buffer( d, d->combat_buffer, 0-leng );
            *d->combat_buffer = '\0';
            d->combat_buffer_length = 0;
            d->combat_buffer_index = 0;
            }

     if( (d->fcommand || ( d->outtop>0 && d->prompter))  &&
           FD_ISSET(d->descriptor, &out_set) )
	    {
      d->prompter=FALSE;

		  if ( !process_output( d, TRUE ) || 
                     IS_SET( d->comm_flags, COMM_FLAG_DISCONNECT))
		    {
		    if ( d->character != NULL && 
                        (d->connected==CON_PLAYING
                        || d->connected==CON_EDITING))
			    save_char_obj(d->character, NORMAL_SAVE);
                *d->outbuf      = '\0';
		    d->outtop	= 0;
		    close_socket( d , TRUE);
		    }
	    }
	  }

  close_timer( TIMER_WRITE_DESC );

      /* Move prompt/tactical updates here  */
  if( first_descriptor!=NULL && !IS_BOOTING)
	for ( d = first_descriptor; d != NULL; d = d_next )
	  {
	  d_next = d->next;
          if( !IS_SET( d->comm_flags, COMM_FLAG_DISCONNECT) &&
              d->character != NULL && 
             (d->connected==CON_PLAYING||d->connected==CON_EDITING) &&
              d->character->pcdata != NULL && 
              d->character->pcdata->tactical_update)
            {
            vt100prompter( d->character );
            d->character->pcdata->tactical_update = FALSE;
            }
          }

  if( first_descriptor!=NULL && !IS_BOOTING)
    for ( d = first_descriptor; d != NULL; d = d->next )
      {
      if(IS_SET( d->comm_flags, COMM_FLAG_DISCONNECT))
        continue;
      write_to_port( d ) ;
      }

        total_io_ticks++;

	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
		  struct timeval stall_time;
	    long secDelta;
	    long usecDelta;
            long secExec;
            long usecExec;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );

	    usecExec 	= ((int) now_time.tv_usec) - ((int) last_time.tv_usec);
	    secExec 	= ((int) now_time.tv_sec ) - ((int) last_time.tv_sec );

    secExec+=usecExec/1000000;
    usecExec=usecExec%1000000;

    secDelta+=usecDelta/1000000;
    usecDelta=usecDelta%1000000;
		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;




    total_io_exec+= secExec*1000 + usecExec/1000;
    if( secDelta>=0 && usecDelta>=0)
        {
             total_io_delay+= secDelta*1000 + usecDelta/1000;
             if( total_io_delay > 100000000)
               {
               total_io_delay/=4;
               total_io_exec/=4; 
               }
		
		/*Removing so Purify can track memory leaks 10/1/99 Martin */
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		  {
		  perror( "Game_loop: select: stall" );
	/*	  abort( );*/
		  }
		
         }
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
  }
 }
}
#endif


void init_descriptor (DESCRIPTOR_DATA *dnew, int desc)
{
    static DESCRIPTOR_DATA d_zero;
    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->outsize	= MAX_STRING_LENGTH;
    CREATE( dnew->outbuf, char, dnew->outsize);
    dnew->back_buf = NULL;
    dnew->timeout = 0;    /* Timeout for blocking descriptor */
    dnew->port_size = 10000;
    dnew->port_baud = 20000;
    dnew->port_timer = 0;
    dnew->outtop = 0;
    *dnew->outbuf      = '\0';
    *dnew->combat_buffer = '\0';
    dnew->combat_buffer_length = 0;
    dnew->combat_buffer_index = 0;
    dnew->creator_location = 0;
    dnew->fork_id = 0;   /*  If not 0 then wait for child to return */
}

#if defined(unix)
void new_descriptor( int control, int port)
{
    /*static DESCRIPTOR_DATA d_zero;*/
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    int desc;
    int size, sockbuf;
    struct linger ld;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY|O_NONBLOCK
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }
 
    sockbuf= 2048;
    if ( setsockopt( desc, SOL_SOCKET, SO_RCVBUF,
        (char *) &sockbuf, sizeof(sockbuf) ) < 0 )
      {
      perror( "new_socket: SO_RCVBUF" );
      }


	ld.l_onoff  = 0;
	ld.l_linger = 100;

	if ( setsockopt( desc, SOL_SOCKET, SO_LINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "new_socket: SO_LINGER" );
	}

    /*
     * Cons a new descriptor.
     */
    CREATE( dnew, DESCRIPTOR_DATA, 1);
    init_descriptor (dnew, desc);

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
	dnew->domain = str_dup( "(unknown)" );
    }
    else
    {   /*  Linked list addition for fast host name lookup  */
	int addr, address[4];
        char buf[MAX_INPUT_LENGTH];

	addr = ntohl( sock.sin_addr.s_addr );

	address[0] = ( addr >> 24 ) & 0xFF ;
        address[1] = ( addr >> 16 ) & 0xFF ;
        address[2] = ( addr >>  8 ) & 0xFF ;
        address[3] = ( addr       ) & 0xFF ;

        sprintf( buf, "%d.%d.%d.%d", address[0], address[1],
                                address[2], address[3]);
        dnew->host = str_dup( buf );
        sprintf( buf, "%d.%d", address[0], address[1]);
        dnew->domain = str_dup( buf );
	sprintf( log_buf, "New connect: %s D%d", dnew->host , dnew->descriptor);
	log_string( log_buf );
    }
    dnew->remote_port=sock.sin_port;
    /*
     * Init descriptor data.
     */
    if ( !last_descriptor && first_descriptor )
    {
        DESCRIPTOR_DATA *d;

        bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
        for ( d = first_descriptor; d; d = d->next )
           if ( !d->next )
                last_descriptor = d;
    }

    LINK( dnew, first_descriptor, last_descriptor, next, prev );

    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some first_person from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Facade: cleaned up code so ban would take affect. 
     */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_suffix( pban->name, dnew->host ) )
	{
	    write_to_descriptor( dnew,
		"Your site has been banned from this Mud.\n\r", 0 );
	    close_socket( dnew , TRUE);
	    return;
	}
    }  

    if(num_descriptors>MAXLINKPERPORT)  /* Limit descriptors  */
    {
      char buf[MAX_STRING_LENGTH];
      sprintf( buf,"\n\rWe have a limit of %d players.\n\r", MAXLINKPERPORT);
      strcat( buf, "Please try back later.\n\r\n\r" );
      write_to_descriptor( dnew, buf, 0);
      close_socket( dnew, TRUE);
      return;
    }


    /*
     * Send the greeting.
     */
      {
      HELP_DATA *pHelp;
      char buf[40];
      int number = number_range(1,4);
      sprintf(buf,"greeting%d",number);
      for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
        {
	if ( is_name( buf, pHelp->keyword ) )
	  {
	  /*
	   * Strip leading '.' to allow initial blanks.
	   */
	  if ( pHelp->text[0] == '.' )
            write_to_buffer( dnew, pHelp->text+1, 0);
	  else
            write_to_buffer( dnew, pHelp->text, 0);
          break;
	  }
        }
      }
    num_descriptors++;
    return;
}
#endif


void close_socket( DESCRIPTOR_DATA *dclose , bool Force)
{
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *dt;
    for( dt=first_descriptor; dt!=dclose && dt!=NULL ; dt=dt->next);
    if( dt==NULL)
      {
      log_string( "Bug: Null desc called in close_socket");
      return;
      }

    write_to_port( dclose);

    if ( dclose->snoop_by != NULL )
      {
      write_to_buffer( dclose->snoop_by,
          "Your victim has left the game.\n\r", 0 );
      dclose->snoop_by = NULL;
      }

    {
      DESCRIPTOR_DATA *d;
      num_descriptors--;
	for ( d = first_descriptor; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( Force && ( ch = dclose->character ) != NULL )
    {
    CHAR_DATA *owner;
   if( ch->desc!=NULL)
     if( ch->desc->original!=NULL)
       {
       DESCRIPTOR_DATA *dtemp;
       dtemp=ch->desc;
       owner=dtemp->original;
       if( IS_NPC(dtemp->character) && dtemp->character->pIndexData->vnum==9900)
         SET_BIT( dtemp->character->act, ACT_WILL_DIE);
       dtemp->character->desc = NULL;
       dtemp->character       = dtemp->original;
       dtemp->original        = NULL;
       ch=owner;
       }

     if( ch != NULL )
       ch->trust = ch->level;

	sprintf( log_buf, "Closing link to %s@%s D%d Connected %d.",
            ch->name!=NULL?ch->name:"NULL",
            dclose->host != NULL ? dclose->host : "NULL" ,
            dclose->descriptor, dclose->connected);
	log_string( log_buf );
        ch->trust = ch->level;  /* Make sure no obj_prog glitches work in */
	if ( dclose->connected == CON_PLAYING 
	||   dclose->connected == CON_EDITING )
	  {
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	  }
	else
          {
          dclose->character = NULL;
          if( !dclose->lookup )
            {
            ch->desc = NULL;
            extract_char( ch , TRUE);
            }
          }
    }


#ifdef UNLINKCHECK
    {
    DESCRIPTOR_DATA *dtemp;
    bool found;
    found=FALSE;
    for( dtemp=first_descriptor;dtemp!=NULL;dtemp=dtemp->next)
      if( dclose==dtemp )
        {
        found=TRUE;
        break;
        }
    if( found )
     UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
    else
     bug( "UNLINK ERROR unlinking descriptor %d.", dclose->descriptor );
    }
#else
     UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
#endif

    close( dclose->descriptor );
    STRFREE (dclose->host );
    STRFREE (dclose->domain );
    if( dclose->back_buf != NULL )
      STRFREE(dclose->back_buf );
    /* fix of memory leak -Dug */
     DISPOSE(dclose->outbuf);
     DISPOSE(dclose);
#if defined(MSDOS) || defined(macintosh)
    abort();
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;
    CHAR_DATA *ch;

    ch = d->original ? d->original : d->character;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;
    if( ch!=NULL && !IS_NPC(ch) && 
       (d->connected == CON_PLAYING ||d->connected == CON_EDITING) && 
            ch->trust != ch->level )
      return( TRUE );

      /* Was there anything here to begin with ? */
    if( d->back_buf != NULL )
      return( TRUE );

    /* Check for overflow. */
    iStart = d->intop;
    if ( iStart >= MAX_INPUT_LENGTH - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d,
	    "\n\r*** PUT A LID ON IT!!! ***\n\rYou have just overflowed your buffer.  You may get back on the game.\n\r", 0 );
        do_save(ch,NULL);
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > MAX_INPUT_LENGTH - 10 )
	/* if ( iStart > sizeof(d->inbuf) - 10 ) */
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;
        char bufin[MAX_STRING_LENGTH];

	nRead = read( d->descriptor, bufin, MAX_INPUT_LENGTH );
        if( nRead > 0 )
          {
          bufin[nRead]='\0';
          d->intop = str_apd_max( d->inbuf, bufin, d->intop, MAX_INPUT_LENGTH );
          }
	else if ( nRead == 0 )
	{
	    /*log_string( "EOF encountered on read." ); */
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
            char buf[80];
            sprintf( buf, "Read_from_descriptor D%d@%s errno %d",d->descriptor,d->host, errno);
            log_string( buf );
	    return FALSE;
	}
    }
#endif

    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;
    CHAR_DATA *ch,*sh;
    char buf[MAX_STRING_LENGTH];


    /*
     * Hold horses if pending command already.
     */
    if ( d==NULL )
      return;
    if ( d->incomm[0] != '\0' )
      return;

    ch=d->original ? d->original : d->character;
    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	  if ( d->inbuf[i] == '\0' )
      {
	    return;
      }
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 40 )
	{
	    write_to_descriptor( d, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
            if( i > MAX_INPUT_LENGTH -2 )
              i=MAX_INPUT_LENGTH -2;
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
            d->intop = i+1;
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii((int)d->inbuf[i]) && isprint((int)d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' || d->incomm[0] == '.' )
    {
    	if ( d->incomm[0] != '.' && d->incomm[0] != '!' &&
        strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{  
       if ( ++d->repeat >= 50 && (d->connected==CON_PLAYING || d->connected==CON_EDITING) &&
           ch->position!=POS_FIGHTING)
            if( d->original == NULL && !IS_NPC( ch ) )
	    {
            if( ch->pcdata->auto_flags == AUTO_OFF )
	      {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d,
		    "\n\r*** PUT A LID ON IT!!! ***\n\rYou have just repeated a command over 50 times.\n\rYou may get back on the game now.\n\r", 0 );
		strcpy( d->incomm, "quit now" );
	      }
            else
              d->repeat = 0;
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( ( d->incomm[0]=='.' || d->incomm[0] == '!' )
        && (d->connected==CON_PLAYING || d->connected==CON_EDITING)
        && d->character!=NULL && ch->alias_ip==0)
      {
      if(d->incomm[1]>='a' && d->incomm[1]<='z')
        str_cpy_max(d->incomm, ch->pcdata->back_buf[(int)(d->incomm[1]-'a')],
                        MAX_INPUT_LENGTH);
      else
      if(d->incomm[1]>='A' && d->incomm[1]<='Z')
        str_cpy_max(d->incomm, ch->pcdata->back_buf[(int)(d->incomm[1]-'A')],
                        MAX_INPUT_LENGTH);
      else
	      strcpy( d->incomm, d->inlast );
      }
    else
      if ( (d->connected==CON_PLAYING || d->connected==CON_EDITING) 
            && d->character!=NULL && d->original==NULL 
            && ch->alias_ip==0 && d->incomm[0]!='\0')
        {
	      strcpy( d->inlast, d->incomm );
        if(d->incomm[0]>='a' && d->incomm[0]<='z')
          {
	  STRFREE(ch->pcdata->back_buf[(int)(d->incomm[0]-'a')] );
	  ch->pcdata->back_buf[(int)(d->incomm[0]-'a')] = str_dup( d->incomm );
          }
        else
        if(d->incomm[0]>='A' && d->incomm[0]<='Z')
          {
	  STRFREE(ch->pcdata->back_buf[(int)(d->incomm[0]-'A')] );
	  ch->pcdata->back_buf[(int)(d->incomm[0]-'A')] = str_dup( d->incomm );
          }
        }
    if(d->snoop_by!=NULL && d->character!=NULL && d->incomm[0]!='\n' &&
       d->snoop_by->character->level>98) /*  Chaos 11/9/93 */
      {
      sh=d->snoop_by->original ? d->snoop_by->original : d->snoop_by->character;
      if (sh!=NULL && sh->desc != NULL && sh->desc->character == sh )
      {
      if (sh->ansi==1)
        {
         sprintf(buf,"\033[1m%s\033[0;36m <%dhp %dmn %dmv> %s",
           ch->name, 
           ch->hit,
           ch->mana,
           ch->move,
           d->incomm );
        }
      else
        {
         sprintf(buf,"%s<%dhp %dmn %dmv>%s",
           ch->name, 
           ch->hit,
           ch->mana,
           ch->move,
           d->incomm );
        }
      write_to_buffer(d->snoop_by,buf,0);
      }
     }

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    d->intop -= i;
    return;
}

void do_copyover (CHAR_DATA *ch, char * argument)
{
        FILE *fp;
        DESCRIPTOR_DATA *d, *d_next;
        char buf [100], buf2[100];

    
        /*if (REAL_GAME)
        {
                send_to_char ("Copyover is not allowed on the real game aborted.\n\r",ch);
                log_string("Could not write to copyover file: ");
                return;
        } */


        fp = fopen (COPYOVER_FILE, "w");

        if (!fp)
        {
                send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
                log_printf ("Could not write to copyover file: %s", COPYOVER_FILE);
                perror ("do_copyover:fopen");
                return;
        }

        /* Consider changing all saved areas here, if you use OLC */

	save_all_clans();
	save_victors();
        /* do_asave (NULL, ""); - autosave changed areas */


        sprintf (buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", ch->name);

        /* For each playing descriptor, save its state */
        for (d = first_descriptor; d ; d = d_next)
        {
                CHAR_DATA * och = CH (d);
                d_next = d->next; /* We delete from the list , so need to save this */

                if (!d->character || d->connected < 0) /* drop those logging on */
                {
                        write_to_descriptor (d, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
                        close_socket (d, TRUE); /* throw'em out */
                }
                else
                {
                        fprintf (fp, "%d %s %s %s\n", d->descriptor, och->name, d->host, d->domain);

                        if (och->level == 1 && is_desc_valid(och))
                        {
                                write_to_descriptor (d, "Since you are level one, and level one characters do not save, you gain a free level!\n\r",
 0);
				och->exp=exp_level(och->class,och->level)+1;
        			och->level += 1;
        			och->mclass[och->class]+=1;
                                advance_level (och, TRUE);
                        }
		
                        save_char_obj(och, NORMAL_SAVE);
                        write_to_descriptor (d, buf, 0);
		
                }
        }

        fprintf (fp, "-1\n");
        fclose (fp);

        /* Close reserve and other always-open files and release other resources
 */

 /*       fclose (fpReserve);*/
        fclose( fpReserve );

        /* exec - descriptors are inherited */

        sprintf (buf, "%d", port);
        sprintf (buf2, "%d", control);
 	if (REAL_GAME)
         execl (REAL_FILE, "Mortal", buf, "copyover", buf2, (char *) NULL);
	else
         execl (TEST_FILE, "Mortal", buf, "copyover", buf2, (char *) NULL);

        /* Failed - sucessful exec will not return */

        perror ("do_copyover: execl");
        send_to_char ("Copyover FAILED!\n\r",ch);

        /* Here you might want to reopen fpReserve */

        if ( ( fpReserve = fopen( NULL_FILE, "r")) == NULL)
	 {
           perror( NULL_FILE );
           abort( );
         }

        if ( ( fpAppend  = fopen( NULL_FILE, "r")) == NULL)
	 {
           perror( NULL_FILE );
           abort( );
         }


}

/* Recover from a copyover - load players */
void copyover_recover ()
{
        DESCRIPTOR_DATA *d;
        FILE *fp;
        char name [100];
        char host[MAX_INPUT_LENGTH], domain[MAX_INPUT_LENGTH];
        int desc;
        bool fOld;


        log_printf ("Copyover recovery initiated");

        fp = fopen (COPYOVER_FILE, "r");

        if (!fp) /* there are some descriptors open which will hang forever then
 ? */
        {
                perror ("copyover_recover:fopen");
                log_printf ("Copyover file not found. Exitting.\n\r");
                exit (1);
        }

        unlink (COPYOVER_FILE);  /* In case something crashes - doesn't prevent reading  */

        for (;;)
        {
		fscanf (fp, "%d %s %s %s\n", &desc, name, host,domain);
                if (desc == -1)
                        break;

                /* Write something, and check if it goes error-free */
/*
                if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r",0))
                {
                        close (desc); * nope *
                        continue;
                }
*/
		CREATE(d, DESCRIPTOR_DATA, 1);
                init_descriptor (d,desc); /* set up various stuff */

                d->host = str_dup (host);
                d->domain = str_dup (domain);
                d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
    	  	LINK( d, first_descriptor, last_descriptor, next, prev );


                /* Now, find the pfile */

                fOld = load_char_obj (d, name);
                if (!fOld) /* Player file not found?! */
                {	
                        write_to_descriptor (d, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
                        close_socket (d, TRUE);
                }
                else /* ok! */
                {
                        /*write_to_descriptor (d, "\n\rCopyover recovery complete.\n\r",0);*/
                        /* Just In Case */
                        if (!d->character->in_room)
                                d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

                        d->connected = CON_PLAYING;
			add_char (d->character);
        		add_player( d->character );
	                if( d->character->vt100!=0 )
       		         vt100prompt( d->character);

        		SET_BIT( d->character->act, PLR_COMBINE );
                        char_to_room (d->character, d->character->in_room);
                        do_look (d->character, "");
                        act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
                }

        }
log_printf("Copyover recovery complete");


}


/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;
    char buf[MAX_STRING_LENGTH];

    if(d==NULL)
      return FALSE;


    /*
     * Bust a prompt.
     */
    if ( fPrompt && !merc_down && 
         (d->connected == CON_PLAYING || d->connected == CON_EDITING))
    {
        CHAR_DATA *ch,*sh;
	ch = d->original ? d->original : d->character;
        if( !is_char_valid( ch ))
          {
          return FALSE;
          }
        sh = d->character;
	if ( IS_SET(ch->act, PLR_BLANK) && ch->vt100!=1)
	    write_to_buffer( d, "\n\r", 2 );

            /*if(d->snoop_by!=NULL)  
              {
              sh2 = d->snoop_by->original ? d->snoop_by->original 
                     : d->snoop_by->character;
              if(sh2->vt100==1)
                sprintf(buf,"%s\033[1m%s", sh->name, 
            prompt_return( sh, IS_NPC( sh2 )?"":sh2->pcdata->prompt_layout ) );
              else
                sprintf(buf,"%s%s", sh->name, 
            prompt_return( sh, IS_NPC( sh2 )?"":sh2->pcdata->prompt_layout ) );
              write_to_buffer( d->snoop_by, buf, 0);
              } */

	if ( ch->alias_ip==0)
        {
            if(ch->vt100!=0 && d->original==NULL && IS_SET(ch->act, PLR_PROMPT))
              vt100prompt(ch);
            else
              {
            strcpy( buf, 
            prompt_return( sh, IS_NPC( ch ) ? "": ch->pcdata->prompt_layout ) );
              if(ch->vt100!=1 || !IS_SET( ch->act, PLR_PROMPT ))
                write_to_buffer( d, buf, 1000000 );
              else
                write_to_buffer( d, buf, 0 );
              }
            if(d->original != NULL && IS_SET( ch->act, PLR_PROMPT ) )
              write_to_buffer( d, "\033[0K", 1000000);
	}

	if ( IS_SET(ch->act, PLR_TELNET_GA) )
	    write_to_buffer( d, go_ahead_str, 1000000 );
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

  return(TRUE);
}

  /* The buffer that works with the page pauser */
int pager( DESCRIPTOR_DATA *d, const char *istr, int lng, char *ostr)
{
  CHAR_DATA *ch;
  int lines,pt, breakpt, lengt;
  bool done;
  char pag_buf[ MAX_BUFFER_LENGTH ];
  register char *ptt, *pto, *pti;
  int lengo;

  open_timer( TIMER_WRITE_PAGER );

  ch=d->original ? d->original : d->character;

  if(IS_NPC(d->character) || 
    ( !IS_NPC(ch) && ch->pcdata->last_command != NULL ) ||
     (d->connected!=CON_PLAYING && d->connected!=CON_EDITING) || 
     IS_SET(ch->act,PLR_PAGER) || ch->pcdata->page_mode!=0 || d->original)
    {
    PAGER_PAUSE=FALSE;
    close_timer( TIMER_WRITE_PAGER );
    return( str_cpy_max( ostr, istr, MAX_STRING_LENGTH ) );
    }
  for(ptt=(char *)istr,pt=0,lines=0;pt<lng;pt++, ptt++)
    if(*ptt=='\n')
      lines++;
  breakpt=(ch->vt100_type%100)-4 -( ch->pcdata->tactical_mode % 100 );
  if(lines<=breakpt+1)
    {
    PAGER_PAUSE=FALSE;
    close_timer( TIMER_WRITE_PAGER );
    return( str_cpy_max( ostr, istr, MAX_STRING_LENGTH ) );
    }
  pt=0;
  lines=0;    /*   Let's recycle this variable  */
  done=FALSE;

  pti = (char *)istr;
  pto = ostr;
  while(!done)
    {
    if(*pti=='\0')
      done=TRUE;
    *pto = *pti;
    if(*pti=='\n')
      lines++;
    if(lines>=breakpt)
      done=TRUE;
    pti++;
    pto++;
    pt++;
    }
  lengo = pt;
  if(*pti=='\r')
    {
    *pto = *pti;
    pto++;
    pti++;
    pt++;
    lengo++;
    }
  *pto = '\0';
  lines=0;
  pto = pag_buf;
  while(pt<lng)
    {
    *pto = *pti;
    pto++;
    pti++;
    pt++;
    lines++;
    }
  *pto = '\0';
  PAGER_PAUSE=TRUE;
  lengt = pt;
  if( ch->vt100!=1 || !IS_SET( ch->act, PLR_PROMPT ) )
    lengt =str_apd_max(ostr,"Press return to continue:\n\r",lengo,
               MAX_STRING_LENGTH); 
  ch->pcdata->page_buf=STRALLOC( pag_buf );
  ch->pcdata->page_mode=1;
  close_timer( TIMER_WRITE_PAGER );
  return( lengt );
}

  /* The buffers that store in the pcdata for the BUFFER and GREP commands */
bool scroll( DESCRIPTOR_DATA *d, const char *txi, bool youcheck, int lng)
  {
  register int cnt;
  CHAR_DATA *ch;
  register bool foundyou;
  int youletter;
  register unsigned char *pti, *pto;
  register int scr_end;
  register bool vt_code;
  register int mr_code;

  foundyou=FALSE;
  youletter=0;

  if((d->connected!=CON_PLAYING && d->connected!=CON_EDITING) 
     || d->original!=NULL)
    return(FALSE);
  ch=d->character;
  if(ch->pcdata->scroll_ip!=0)
    return(FALSE);

  open_timer( TIMER_WRITE_SCROLL );

    pto = ch->pcdata->scroll_buf + ch->pcdata->scroll_end;
    pti = (char *)txi;
    scr_end = ch->pcdata->scroll_end;
    vt_code = FALSE;
    mr_code = 0;

  for(cnt=0;cnt<lng;cnt++, pti++)
    {
    if( youcheck && !vt_code && mr_code==0)
      {
      if(foundyou==FALSE)
        {
        if(*pti=='Y' || *pti=='y')
          {
          foundyou=TRUE;
          youletter=1;
          }
        }
      else
        if( youletter==1)
        {
          if( *pti=='o' || *pti=='O')
            youletter=2;
          else
            foundyou=FALSE;
        }
        else
         if( youletter==2)
         {
          if( *pti=='u' || *pti=='U')
            {
            youletter=3;
            youcheck = FALSE;
            }
          else
            foundyou=FALSE;
         }
      }
    if( !vt_code && *pti == '\033' && *(pti+1)=='[' )
      {
      vt_code = TRUE;
      continue;
      }
    if( vt_code )
      {
      if( (*pti>='a' && *pti<='z') || (*pti>='A' && *pti<='Z'))
        vt_code = FALSE;
      continue;
      }
    if( mr_code == 0 && (*pti==14 || *pti==15 || *pti==16 ||
        *pti==20 || *pti==18))
      {
      if( *pti==14 || *pti==15 )
        mr_code=1;
      else
      if( *pti==16 )
        mr_code=2;
      else
      if( *pti==20 )
        mr_code=3;
      else
      if( *pti==18 )
        mr_code=4;
      continue;
      }
    if( mr_code > 0 )
      {
      mr_code--;
      continue;
      }
    if( *pti<32 || *pti>127)
      {
      if( *pti=='\0' )
        {
        ch->pcdata->scroll_end=scr_end;
        if( youletter == 3 )
          return( TRUE );
        else
          return( FALSE );
        }
      if(*pti!='\n' && *pti!='\r')
        continue;
      }
    *pto = *pti;
    pto++;
    scr_end ++;
    if(scr_end == MAX_BUFFER_LENGTH)
      {
      ch->pcdata->scroll_start=1;
      ch->pcdata->scroll_end=0;
      scr_end = 0;
      pto = ch->pcdata->scroll_buf;
      }
    }
/*
  if( *pto!='\r' && *pto!='\n' )
    {
    *pto = '\n';
    pto++;
    scr_end ++;
    if(scr_end == MAX_BUFFER_LENGTH)
      {
      ch->pcdata->scroll_start=1;
      ch->pcdata->scroll_end=0;
      scr_end = 0;
      pto = ch->pcdata->scroll_buf;
      }
    *pto = '\r';
    pto++;
    scr_end ++;
    if(scr_end == MAX_BUFFER_LENGTH)
      {
      ch->pcdata->scroll_start=1;
      ch->pcdata->scroll_end=0;
      scr_end = 0;
      pto = ch->pcdata->scroll_buf;
      }
    }
*/

  ch->pcdata->scroll_end=scr_end;

  close_timer( TIMER_WRITE_SCROLL );

  if( youletter == 3 )
    return( TRUE );
  else
    return( FALSE );
  }

char *ansi_strip(char *txi)
{
  register unsigned char *pti, *pto;
  register bool vt_code;
  register int mr_code;

    pti = (char *)txi;
    pto = (char *)ansi_strip_txt;
    vt_code = FALSE;
    mr_code = 0;

  for(;*pti!='\0';pti++)
    {
    if( !vt_code && *pti == '\033' && *(pti+1)=='[' )
      {
      vt_code = TRUE;
      continue;
      }
    if( vt_code )
      {
      if( (*pti>='a' && *pti<='z') || (*pti>='A' && *pti<='Z'))
        vt_code = FALSE;
      continue;
      }
    if( mr_code == 0 && (*pti==14 || *pti==15 || *pti==16 ||
        *pti==20 || *pti==18))
      {
      if( *pti==14 || *pti==15 )
        mr_code=1;
      else
      if( *pti==16 )
        mr_code=2;
      else
      if( *pti==20 )
        mr_code=3;
      else
      if( *pti==18 )
        mr_code=4;
      continue;
      }
    if( mr_code > 0 )
      {
      mr_code--;
      continue;
      }
    if( *pti<32 || *pti>127)
      {
      if(*pti!='\n' && *pti!='\r')
        continue;
      }
    *pto = *pti;
    pto++;
    }
  *pto='\0'; 
  return( ansi_strip_txt );
}
/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, char *txt, int length )
{
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char txo[MAX_STRING_LENGTH];
  CHAR_DATA *ch,*sh;
  int cnt, leng, lengt;
  bool NO_PAGE;

  if( length < 0 )
    {
    length = 0-length;
    NO_PAGE = TRUE;
    }
  else
    NO_PAGE = FALSE;
    
  if(d==NULL || txt==NULL)
    return;

  if( !ALLOW_OUTPUT && length != 1000000 )
    return;


    /* Let's fix that combat buffer up */
  if( d->combat_buffer_length > 0  && !NO_PAGE)
    {
    leng = d->combat_buffer_length;
    write_to_buffer(d, d->combat_buffer, 0-leng);
    *d->combat_buffer = '\0';
    d->combat_buffer_length = 0;
    d->combat_buffer_index = 0;
    }

  ch = d->original ? d->original : d->character;
    /* Kill the swear words  Presto 7/98
       I still think this sucks but if we have to it, it's configurable 
       Martin 9/9/98 

       Chaos - This code is too inefficient here.  9/13/98

    if( ch!=NULL && length!=1000000 &&
       !IS_NPC(ch) && !IS_SET(ch->pcdata->player2_bits, PLR2_CENSOR ))
      censorstring(txt);  */

  if(ch!=NULL)
    if(ch->vt100==1)
    {
    if(txt[0]=='\n' && (txt[1]=='\0' || txt[2]=='\0' ||(length>0 && length<3)))
      return;
    if(txt[0]=='\r' && (txt[1]=='\0' || txt[2]=='\0' ||(length>0 && length<3)))
      return;
    if(txt[0]=='\0') 
      return;
    }
  lengt=length;
  if(ch!=NULL)
    {
    if( is_desc_valid( ch))
      ch->desc->prompter=TRUE;
    if(ch->vt100==0 && d->outtop == 0 && !d->fcommand)
      {
      d->outbuf[0] = '\n';
      d->outbuf[1] = '\r';
      d->outbuf[2] = '\0';
      d->outtop = 2;
      }
    if(ch->vt100==1 && length!=1000000)
      {
      bool foundyou;
      if(lengt<=0)
        lengt=strlen(txt);
      if( lengt > MAX_STRING_LENGTH-16)
        lengt = MAX_STRING_LENGTH-16;
      if( NO_PAGE )
        {
        lengt = str_cpy_max( txo, txt, lengt );
        foundyou = FALSE;
        }
      else
        {
        lengt = pager(d,txt,lengt,txo);
        foundyou=scroll( d, txo, (ch->vt100_type/1000%10==1), lengt);
        }
      if(ch->vt100_type/100%10==0)
        cnt=ch->vt100_type%100-1;
      else
        cnt=ch->vt100_type%100-2;
      if( !IS_SET( ch->act, PLR_PROMPT ))
        {
        *buf = '\0';
        leng = 0;
        }
      else
        {
        leng = str_cpy_max( buf, vt_command_0( ch, 'K' ), MAX_STRING_LENGTH );
        leng = str_apd_max( buf, "\0337", leng, MAX_STRING_LENGTH );
        leng = str_apd_max( buf, vt_command_1( ch, cnt, 'H' ), leng, 
                                             MAX_STRING_LENGTH );
        }
      
      if(foundyou)
        {
        if( ch->ansi!=0 )
          {
       leng=str_apd_max( buf, get_color_string( ch, COLOR_TEXT, VT102_BOLD ),
                    leng, MAX_STRING_LENGTH );
          }
        else
          leng = str_apd_max(buf,"\033[1m", leng, MAX_STRING_LENGTH );
        }
      else
        {
        if( ch->ansi!=0 )
          {
        leng =str_apd_max( buf, get_color_string( ch, COLOR_TEXT, VT102_DIM ),
                    leng, MAX_STRING_LENGTH );
          }
        else
          leng = str_apd_max(buf,"\033[m", leng, MAX_STRING_LENGTH );
        }
          
      leng = str_apd_max( buf, txo, leng, MAX_STRING_LENGTH );
      if(buf[leng-1]!='\n' && buf[leng-1]!='\r')
        {
         if (ch->desc)
         {
          if (ch->desc->connected !=CON_EDITING)
           leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
          else if (txt[strlen(txt)-2]!='>')
           leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
          }
         else
          leng = str_apd_max( buf, "\n\r", leng, MAX_STRING_LENGTH );
        }
      if( IS_SET( ch->act, PLR_PROMPT ))
        {
/* I added this in to explicitly reset cursor colour instead
of relying on highly dodgy \0337 and \0338 codes which only remember
position and not colour...Martin*/

        lengt =  str_apd_max(buf,"\0338", leng, MAX_STRING_LENGTH); 
        if (ch->ansi !=0 && ch->pcdata->color[0]!=0)
            sprintf(buf2, "\033[0;%d;%dm", ch->pcdata->color[0][0],
		ch->pcdata->color[1][0]);
        else buf2[0]='\0';

        lengt = str_apd_max(buf, buf2, lengt, MAX_STRING_LENGTH);
        lengt = str_apd_max(buf,"\033[0K", lengt, MAX_STRING_LENGTH);

        if( ch->vt100 == 1 && PAGER_PAUSE)
          {
          lengt = str_apd_max( buf, "<Press return to continue>", lengt, 
                     MAX_STRING_LENGTH );
          }
        }
      else
        if( ch->vt100 == 1 )
          lengt = str_apd_max( buf, "\033[0m", leng, MAX_STRING_LENGTH );

      }
   else
      {
      if(lengt<=0 || length==1000000)
        lengt = strlen(txt);
      if( lengt > MAX_STRING_LENGTH-8)
        lengt = MAX_STRING_LENGTH-8;
      lengt = pager(d,txt,lengt,txo);
      if(length!=1000000)
        scroll( d, txo, FALSE, lengt);
      str_cpy_max( buf, txo, MAX_STRING_LENGTH );
      if(lengt>1 && length!=1000000)   /* Chaos 11/9/93  */
        if(buf[lengt-1]!='\n' && buf[lengt-1]!='\r')
         if (ch->desc)
         {
          if (ch->desc->connected!=CON_EDITING)
           lengt = str_cat_max(buf,"\n\r", MAX_STRING_LENGTH);
          else if (txt[strlen(txt)-2]!='>')
           lengt = str_cat_max( buf, "\n\r", MAX_STRING_LENGTH );
         }
      }
   }
else
      {
      if(d->outtop==0 && !d->fcommand)
        {
        d->outbuf[0]='\n';
        d->outbuf[1]='\r';
        d->outbuf[2]='\0';
        d->outtop = 2;
        }
      lengt = str_cpy_max(buf, txt, MAX_STRING_LENGTH);
      /* if(length<=0||length==1000000)
        lengt=strlen(buf);
      else
        buf[lengt]='\0'; */
      }

if(ch!=NULL && !DISALLOW_SNOOP )
  if(d->snoop_by!=NULL && length!=1000000)   /* Chaos 11/9/93 */
    {
    sh = d->snoop_by->original ? d->snoop_by->original : d->snoop_by->character;
    if( sh!=NULL && sh->desc != NULL && sh->desc->character == sh )
      {
      if(sh->ansi==1)
        {
         sprintf(buf3,"\033[1m%s\033[0;36m <%dhp %dmn %dmv> %s",
           ch->name, 
           ch->hit,
           ch->mana,
           ch->move,
           ansi_strip( txt ) );
        }
      else
        {
         sprintf(buf3,"%s<%dhp %dmn %dmv>%s",
           ch->name, 
           ch->hit,
           ch->mana,
           ch->move,
           ansi_strip( txt ) );
        }
      write_to_buffer(d->snoop_by, buf3, 0);
      }
    }
  /*
   * Copy.
   */
  open_timer( TIMER_WRITE_APPEND );
  d->outtop = str_apd_max( d->outbuf , buf, d->outtop, d->outsize );
  /* d->outtop += lengt; */
  close_timer( TIMER_WRITE_APPEND );

  return;
}



/*
 * Lowest level buffer function.
 */
bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
{

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    d->outtop = str_apd_max( d->outbuf, txt, d->outtop, MAX_STRING_LENGTH );

    write_to_port( d );

    return TRUE;
}


/*                                Chaos    5/17/95
 * Write a block of text to the file descriptor.
 * Utilizing the PORT size feature.
 * Used to flush the outbuf buffer to the actual socket.
 */
void write_to_port( DESCRIPTOR_DATA *d )
{
    int nWrite;
    int nBlock;
    char *pt, *pto;
    int length, clen;
    char buf[80];
    int twrite, cnt;
    bool failure;


#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    length = d->outtop;
    clen = length;
    pt = d->outbuf;
    nWrite = 1;
    twrite=0;
    failure = FALSE;


    while( clen > 0  && nWrite > 0 && !failure)
       {

        nBlock = UMIN( clen, d->port_size );

        if ( ( nWrite = write( d->descriptor, pt, nBlock ) ) < 1 ) 
          {
          if( errno != EAGAIN )  /* Skip normal delays */
	    { 
            sprintf( buf, "Write_to_descriptor D%d@%s ", d->descriptor,d->host);
            perror( buf );
            d->outtop = 0;
            *d->outbuf='\0';
            SET_BIT( d->comm_flags , COMM_FLAG_DISCONNECT);
            return;
            }
          failure = TRUE;
          }

     if( nWrite > 0 )
       {
        clen   -= nWrite;
        pt     += nWrite;
        twrite += nWrite;
       }

      }

        /*if( nWrite == 0 )
            {
            sprintf( buf, "Write of 0 length D%d@%s ", d->descriptor,d->host);
            perror( buf );
            } */

  total_io_bytes+=twrite;

  if( total_io_bytes > 200000)
    { total_io_bytes/=3; total_io_ticks/=3;}

  if( failure )
    {
    d->timeout++;
    if( d->timeout > 60 )  /* 30 sec timeout on buffers */
	    { 
            sprintf( buf, "Write_to_descriptor: timeout D%d@%s ", d->descriptor,d->host);
            log_string( buf );
            d->outtop = 0;
            *d->outbuf='\0';
            d->timeout = 0;
            SET_BIT( d->comm_flags , COMM_FLAG_DISCONNECT);
            return;
            }
    return;
    }

  d->timeout = 0;

  if( twrite == length )
    {
    d->outtop = 0;
    *d->outbuf='\0';
    }
  else
    {
    cnt=length-twrite;
    pto = d->outbuf;
    for( ;cnt>0; cnt--, pt++, pto++)
      *pto=*pt;
    *pto='\0';
    d->outtop = length-twrite;
    }

    return;
}

  /*  Chaos - 3/21/99 */
void display_class_selections( DESCRIPTOR_DATA *d)
  {
  char buf[MAX_INPUT_LENGTH];
  char buft[MAX_INPUT_LENGTH];
  CHAR_DATA *ch;
  int iClass;

  ch = d->character;

  strcpy( buf, "{030}You may choose from the following classes, or type help [class] to learn more:\n\r{160}     " );

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
          if( race_table[ch->race].race_class[iClass]==0)
	    {
            sprintf( buft, "%s     ", class_table[iClass].who_name_long);
            strcat( buf, buft );
            }

	strcat( buf, "\n\r{030}Please choose a class: {110}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
  return;
  }

  /*  Chaos - 3/21/99 */
void display_race_details( DESCRIPTOR_DATA *d )
  {
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *ch;

  ch = d->character;

        if( ch->vt100 == 0 )
          {
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          }
        else
          write_to_buffer( d, "\033[2J\033[H", 1000000);

          strcpy( buf, "{030}Details on your race selection:\n\r" );
    	  write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        sprintf( buf, "{060}Race: {130}%-10s      {060}Speed:{130} ", 
            race_table[ch->race].race_name);
        switch( race_table[ch->race].max_speed )
          {
          case 0: strcat( buf, "Walk  "); break;
          case 1: strcat( buf, "Normal"); break;
          case 2: strcat( buf, "Jog   "); break;
          case 3: strcat( buf, "Run   "); break;
          case 4: strcat( buf, "Haste "); break;
          }
        switch( race_table[ch->race].vision )
          {
          case 0: strcat( buf, "  {060}Vision: {130}Normal\n\r"); break;
          case 1: strcat( buf, "  {060}Vision: {130}Night\n\r"); break;
          case 2: strcat( buf, "  {060}Vision: {130}Dark\n\r"); break;
          }
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        
        sprintf( buf, "{060}Str:{130}%2d   {060}Dex:{130}%2d   {060}Int:{130}%2d   {060}Wis:{130}%2d   {060}Con:{130}%2d   {060}HP:{130}%2d   {060}Mana:{130}%2d   {060}Move:{130}%2d\n\r",
            race_table[ch->race].race_mod[0],
            race_table[ch->race].race_mod[1],
            race_table[ch->race].race_mod[2],
            race_table[ch->race].race_mod[3],
            race_table[ch->race].race_mod[4],
            race_table[ch->race].hp_mod,
            race_table[ch->race].mana_mod,
            race_table[ch->race].move_mod );
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        sprintf( buf, "{060}Special Ability:  {130}%s\n\r", 
            race_table[ch->race].race_special );
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);

  return;
  }

  /*  Chaos - 3/21/99 */
void display_race_selections( DESCRIPTOR_DATA *d )
{
  char buf[MAX_INPUT_LENGTH];
  char buft[MAX_INPUT_LENGTH];
  sh_int iRace;
  CHAR_DATA *ch;

    ch = d->character;


      strcpy( buf, "         {130}Race Selections\n\r" );
      strcat( buf, "{030}You may choose from the following races, or type help [race] to learn more:\n\r" );
      write_to_buffer( d, (char *)ansi_translate_text(ch, buf), 1000000);
      strcpy( buf, "{160}    " );
	for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	  {
          sprintf( buft, "%-11s ", race_table[iRace].race_name);
          strcat( buf, buft );
          if( iRace == 5 )
            strcat( buf, "\n\r{160}    " );
          }
      write_to_buffer( d, (char *)ansi_translate_text(ch, buf), 1000000);
      strcpy( buf, "\n\r{030}Please choose a race: {110}" );
      write_to_buffer( d, (char *)ansi_translate_text(ch, buf), 1000000);
  return;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
bool nanny( DESCRIPTOR_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    CHAR_DATA *ch, *rch;
    char *pwdnew;
    /* char *p;  */
    int iClass,iRace, cnt,tst;
    bool fOld;
    bool outp;

    outp=FALSE;
    while ( isspace((int)*argument) )
	argument++;

    ch = d->character;

    if( d->connected >= CREATOR_MAIN )
      {
      outp = creator( d, argument);
      if( !outp )
        display_creator( d );
      return( outp );
      }
    else
    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );	
	close_socket( d , TRUE);
	return(outp);

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d , TRUE);
	    return(outp);
	}
        /* for (cnt=0;cnt<strlen(argument);cnt++)
         argument[cnt]=LOWER(argument[cnt]); */

	argument[0] = UPPER(argument[0]);
        
	if ( !check_parse_name( argument, FALSE ) )
	  {
          write_to_buffer( d, "Illegal name, try another.\n\rName: ", 1000000 );
	  return(outp);
	  }

  ch=lookup_char( argument );  /* see if this char is on game now playing */
                             /* We don't care if they are not quite connected */


  d->character = NULL;
  fOld = FALSE;
  remove_bad_desc_name( argument );
  if( ch==NULL || ch->pcdata==NULL )   /* mobile ? */
    {
          load_error = FALSE;
	  fOld = load_char_obj( d, argument );
	  ch   = d->character;
          d->lookup=FALSE;
          ch->pcdata->page_mode=0;
	  if ( wizlock && !IS_HERO(ch) )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d , TRUE);
		return(outp);
	    }
         if( ch->name == NULL || strcasecmp( ch->name, argument ) )
           {
		write_to_buffer( d, "Your character is faulty.  Contact the Gods.\n\r", 0 );
		close_socket( d , TRUE);
		return(outp);
	    }
    }
  else
    {
    fOld=TRUE; 
    d->character=ch;
    d->lookup=TRUE;
    }

  if( ch->name == NULL || ch->pcdata == NULL )
    {
    log_string( "Found nullified character" );
    close_socket( d , TRUE);
    return(outp);
    }

   /* Check here for tintin thrashing 
  if (!TEST_GAME)
  {
  int ntime;

  ntime = get_game_realtime();
   if( fOld )
     {
     if( ch->pcdata->last_connect != 0  && ntime > ch->pcdata->last_connect &&
         ntime - ch->pcdata->last_connect < 30 && ch->level<=95)
       {

       write_to_buffer( d, "You are logging in too quick.  Please wait 1 minute.\n\r", 1000000);
        if( !fOld )
          ch->wait = PULSE_TICK;
        else
          {
	  close_socket( d , TRUE);
	  return(outp);
          }
       }
     }
   ch->pcdata->last_connect = ntime;
  }
  */   
	if ( IS_SET(ch->act, PLR_DENY) && !TEST_GAME)
	{
          if( ch->level >= 99 )
	    sprintf( log_buf, "Denying access to GOD %s@%s.", argument, d->host );
          else
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d , TRUE);
	    return(outp);
	}

        if( !strcasecmp( argument, "new" ) )
          {
	    free_char( d->character );
	    d->character = NULL;
	    write_to_buffer( d, "\n\rChoosing a name is one of the most important parts of this game...\n\r"
                                "Make sure to pick a name appropriate to the character you are going\n\r"
                                "to role play, and be sure that it suits a medieval theme.\n\r"
                                "If the name you select is not acceptable, you will be asked to choose\n\r"
                                "another one.\n\r\n\rPlease choose a name for your character: ", 1000000);

          d->connected = CON_GET_NEW_NAME;
          return( outp );
          }

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 1000000 );
	    write_to_buffer( d, echo_off_str, 1000000 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return(outp);
	}
	else
	{
	if ( !check_parse_name( argument, FALSE ) )
	  {
          write_to_buffer( d, "Illegal name, try another.\n\r\n\rName: ", 1000000 );
	  return(outp);
	  }
	    /* New player */
	    write_to_buffer( d, "\n\rYou may not choose a new name now.\n\rIf you wish to create a new character, enter a name of 'NEW'\n\r\n\rName: ", 1000000 );
	    free_char( d->character );
	    d->character = NULL;
	    return(outp);
	}
	break;

      case CON_GET_NEW_NAME:
  if( !strcasecmp( argument, "new" ) )
	  {
          write_to_buffer( d, "\n\rIllegal name, try another.\n\r\n\rName: ", 1000000 );
	  return(outp);
	  }
    

  ch=lookup_char( argument );  /* see if this char is on game now playing */
                             /* We don't care if they are not quite connected */
  d->character = NULL;
  remove_bad_desc_name( argument );

  fOld = FALSE;
  if( ch!=NULL)
    fOld = TRUE;

  if( !fOld  )
    {
    load_error = FALSE;
    fOld = load_char_obj( d, argument );
    ch = d->character;
    }

	  if ( wizlock )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d , TRUE);
		return(outp);
	    }

  if( fOld )
    {
    if( d->character != NULL )
      {
      free_char( d->character );
      }
    d->character = NULL;
    ch = NULL;
    write_to_buffer( d, "\n\rYou have chosen a name that already exists.  Please try another.\n\r\n\rNew name: ", 1000000 );
    return( outp );
    }

	if ( !check_parse_name( argument, TRUE ) )
	  {
          write_to_buffer( d, "Illegal name, try another.\n\r\n\rNew name: ", 1000000 );
	  return(outp);
	  }

    if( *ch->name >= 'a' && *ch->name <='z' )
      *ch->name -= ('a' - 'A') ;

	    /* New player */
	    sprintf( buf, "\n\rYou must now choose a password.  This password must contain at least five\n\rcharacters, with at least one of them being a number.\n\rMake sure to use a password that won't be easily guessed by someone else.\n\r\n\rGive me a good password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 1000000 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;
         

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

  if( ch->name == NULL || ch->pcdata == NULL )
    {
    wipe_string( argument );
    log_string( "Found nullified character" );
    close_socket( d , TRUE);
    return(outp);
    }
	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "\n\rWrong password.\n\r", 1000000 );
            wipe_string( argument );
            if( d->lookup )
	      close_socket( d , FALSE);
            else
	      close_socket( d , TRUE);
	    return(outp);
	}
       if( !is_valid_password( argument ) )
         ch->pcdata->unsafe_password = TRUE;

       wipe_string( argument );

        /* Multiplaying limiter test   - Removed  7/15/96
       if( count_links( d->host_index ) > d->host_index->max_connect )
         {
         DESCRIPTOR_DATA *dtemp;
         strcpy( buf, "The following players are connected to the game:\n\r" );
         for( dtemp=first_descriptor;dtemp!=NULL;dtemp=dtemp->next )
           if( dtemp!=d && dtemp->host_index == d->host_index )
             {
             sprintf( buf2, "%s\n\r", dtemp->character!=NULL?
                    capitalize(dtemp->character->name):"unknown" );
             strcat( buf, buf2 );
             }
         sprintf( buf2, "The system will not allow more than %d players connect from your site.\n\rPlease contact one of these players, or try connecting later.\n\r", d->host_index->max_connect );
         strcat( buf, buf2 );
         write_to_buffer( d, buf, 1000000 );
            wipe_string( argument );
         if( d->lookup )
	   close_socket( d , FALSE);
         else
	   close_socket( d , TRUE);
	 return(outp);
         }  */

           /* Determine if it's a new guy over limit */
       if( total_characters >= MAXLINKPERPORT && !d->lookup && 
            ch!=NULL && !IS_IMMORTAL(ch))
         {
	 write_to_buffer( d, "\n\rThe game currently has the maximum amount of players online\n\rTry back in a few minutes.\n\r", 1000000 );
	 close_socket( d , TRUE);
         return(outp);
         }

	write_to_buffer( d, echo_on_str, 0 );
  
  if( check_reconnecting( d, ch ) )
    {
    usage.numRecons+=1;
    sprintf( log_buf, "%s@%s has reconnected.  D%d", ch->name, d->host,
        d->descriptor );
    log_string( log_buf );
    ch->desc = d;
    if(ch->vt100!=0 )
      {
      ch->vt100=2;
      if( IS_SET( ch->act, PLR_PROMPT))
        vt100prompt(ch);
      else
        vt100on(ch);
      }
    send_to_char( "Reconnecting.\n\r",  ch);
    if( !find_ch_game( ch ))
      {
      d->connected = CON_READ_MOTD;
      send_to_char( "Press return:\n\r", ch);
      }
    else
      d->connected = CON_PLAYING;
    d->character = ch;
    d->incomm[0]='\0';
    outp=TRUE;
    }
  else
    {
    ch=d->original ? d->original : d->character;
    ch->desc=d;
	  sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	  log_string( log_buf );
    /*if( ch->vt100!=0 && ch->ansi==1 && ch->pcdata->term_info!=0 )
      {
      sprintf(buf,"%c%c%c", 14, 24, 32 + ch->vt100_type %100 );
      write_to_descriptor(d,buf,0);
      } */
    if(ch->vt100!=0)
      {
      if( IS_SET( ch->act, PLR_PROMPT))
        vt100prompt(ch);
      else
        vt100on(ch);
      }
    do_help( ch, "motd");
	  d->connected = CON_READ_MOTD;
    scan_for_duplication( ch );
    }

  d->lookup = FALSE;

  {
  int count;
   for( cnt=0, count=0; cnt<MAX_CLASS; cnt++)
     count+=d->character->mclass[cnt];
   if(count!=d->character->level && d->character->level<97 && 
          d->character->level>10)
      {
      write_to_buffer( d, "You character has some serious flaws.  Please talk to the gods about this\n\rproblem.  You will not be able to use this character until it is fixed.\n\r", 1000000);
      sprintf( log_buf, "%s had faulty character.", d->character->name);
      log_string( log_buf );
      close_socket( d, TRUE);
      }
}


	break;


    case CON_VT100:
        switch ( *argument )
        {
        case 'a': case 'A':
           ch->ansi=1;
           ch->vt100=2;
           ch->vt100_type -= ch->vt100_type%100;
           ch->vt100_type += 24;
	   ch->pcdata->tactical_mode=4;
           reset_color( ch );
	   d->connected = CON_GET_NEW_SEX;
           break;

        case 'v': case 'V':
           ch->vt100=2;
           ch->ansi=0;
           ch->vt100_type -= ch->vt100_type%100;
           ch->vt100_type += 24;
	   ch->pcdata->tactical_mode=4;
           reset_color( ch );
	   d->connected = CON_GET_NEW_SEX;
           break;

        case 'm': case 'M':
          ch->vt100=2;
          ch->ansi=1;
          ch->pcdata->term_info = 1;
          ch->vt100_type -= ch->vt100_type%100;
          ch->vt100_type += 24;
          ch->act += PLR_TERMINAL;
          reset_color( ch );
	  d->connected = CON_GET_NEW_SEX;
           break;

        case 't': case 'T':
           ch->vt100 = 0;
          ch->ansi=0;
	  d->connected = CON_GET_NEW_SEX;
           break;

        default:
          write_to_buffer( d, "\n\rThat was not a terminal type.\n\r\n\rPlease choose a terminal type: ", 1000000);
          break;
        }

      if( d->connected == CON_GET_NEW_SEX )
        {
        if( ch->vt100 == 0 )
          {
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          }
        else
          write_to_buffer( d, "\033[2J\033[H", 1000000);

        sprintf( buf, "\n\r{060}Please choose a sex for your player.\n\r\n\r{130}    M{060} - Male\n\r{130}    F{060} - Female\n\r\n\r{030}Select your sex: {110}" );
        write_to_descriptor( d, (char *)ansi_translate_text(ch,buf), 0 );
        }
        
      break;


           
    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "\n\rYou must now choose a password.  This password must contain at least five\n\rcharacters, with at least one of them being a number.\n\rMake sure to use a password that won't be easily guessed by someone else.\n\r\n\rGive me a good password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 1000000 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 1000000 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 1000000 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"\n\rPassword must be at least five characters long.\n\rPassword: ",
		1000000 );
	    return(outp);
	}

        if( !is_valid_password( argument ) )
	    {
	    write_to_buffer( d,
	      "\n\rThat password is not acceptable, try again.\n\rPasswords may only contain letters (case sensitive), or numbers.\n\rAt least one number is required in the password.\n\rPassword: ",
		    1000000 );
		return(outp);
            }

	pwdnew = crypt( argument, ch->name );

        wipe_string( argument );

	STRFREE(ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "\n\rPlease retype the password to confirm: ", 1000000 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "\n\rPasswords don't match.\n\rRetype password: ",
		1000000 );
	    d->connected = CON_GET_NEW_PASSWORD;
        wipe_string( argument );
	    return(outp);
	}

        wipe_string( argument );

   write_to_buffer( d, "\n\r", 1000000);
	 write_to_buffer( d, echo_on_str, 0 );

    write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
    write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
    write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
    write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
    write_to_buffer( d, "Selecting your terminal type is very important in Mortal Realms.  Choose the\n\rthe configuration that will give you the most features.\n\r\n\r", 1000000 );
    write_to_buffer( d, "M - MrTerm users.  Gives best graphical performance.\n\r", 1000000 );
    write_to_buffer( d, "A - ANSI color with VT102 controls.  Use on all color systems.\n\r", 1000000 );
    write_to_buffer( d, "V - VT102 compatible.  Use on monochrome terminals.\n\r", 1000000 );
    write_to_buffer( d, "T - TTY.  Suggested use with tintin or non-VT102 compatible terminals.\n\r", 1000000 );
    write_to_buffer( d, "\n\rChoose a terminal type:", 1000000 );
	    d->connected = CON_VT100;
	    break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	default:
            if( ch->vt100 == 0 )
              {
              write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
              write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
              write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
              write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
              }
            else
              write_to_buffer( d, "\033[2J\033[H", 1000000);
            sprintf( buf, "\n\r{130}That's not a sex.\n\rPlease choose a sex for your player.\n\r{110}M{060} - Male\n\r{110}F{060} - Female\n\r{170}Select your sex: {110}" );
	    write_to_buffer( d, (char *)ansi_translate_text(ch, buf),
              1000000 );
	    return(outp);
	}

        if( ch->vt100 == 0 )
          {
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          }
        else
          write_to_buffer( d, "\033[2J\033[H", 1000000);
        display_race_selections( d );

	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
        argument = one_argument(argument, buf2);
        if (!strcasecmp( buf2, "help") )
        {
          for ( iRace = 0; iRace < MAX_RACE; iRace++ )
          {
            if ( toupper(argument[0]) == toupper(race_table[iRace].race_name[0])
            &&  !str_prefix( argument, race_table[iRace].race_name) )
            {
              ch->race = iRace;
              display_race_details( d );
    	      write_to_buffer( d, "\n\r", 1000000);
              strcpy( buf, race_table[iRace].race_name );
              strcat( buf, "intro" );
              do_help(ch, buf);
    	      write_to_buffer( d, "\n\r", 1000000);
  	      display_race_selections ( d );
              return(outp);
            }
          }
          strcpy( buf, "\n\r{010}No help on that topic.\n\r\n\r{030}Please choose a race: {110}" );
    	  write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
          return (outp);
        }
 
        for( iRace=0; iRace<MAX_RACE;iRace++)
        {
            if ( toupper(buf2[0]) == toupper(race_table[iRace].race_name[0])
            &&   !str_prefix( buf2, race_table[iRace].race_name ) )
            {
                ch->race = iRace;
                break;
            }
 	}
        if( iRace>=MAX_RACE )
          {
          strcpy( buf, "\n\r{010}That's not a race.\n\r\n\r{030}Please choose a race: {110}" );
    	  write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
	    return(outp);
	  }



        display_race_details( d );
    	      write_to_buffer( d, "\n\r", 1000000);
        display_class_selections( d );

	d->connected = CON_GET_NEW_CLASS;
	break;


    case CON_GET_NEW_CLASS:
        argument = one_argument(argument, buf2);
        if (!strcasecmp( buf2, "help") )
        {
          for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
          {
            if ( toupper(argument[0]) == toupper(class_table[iClass].who_name_long[0])
            &&  !str_prefix( argument, class_table[iClass].who_name_long) )
            {
        if( ch->vt100 == 0 )
          {
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          }
        else
          write_to_buffer( d, "\033[2J\033[H", 1000000);
	strcpy( buf, "{060}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
              strcpy( buf, class_table[iClass].who_name_long );
              strcat( buf, "intro" );
              do_help(ch, buf);
        write_to_buffer( d, "\n\r", 1000000);
        display_class_selections( d );
              return(outp);
            }
          }
	strcpy( buf, "\n\r\n\r{060}No help on that topic.\n\r{030}Please choose a class: {110}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
          return (outp);
        }
 
        for( iClass=0; iClass<MAX_CLASS;iClass++)
        {
            if ( toupper(buf2[0]) == toupper(class_table[iClass].who_name_long[0])
            &&   !str_prefix( buf2, class_table[iClass].who_name_long )
            &&   race_table[ch->race].race_class[iClass]==0)
            {
                ch->class = iClass;
                break;
            }
 	}

	if ( iClass == MAX_CLASS )
	{
	strcpy( buf, "\n\r\n\r{060}That is not a class.\n\r{030}Please choose a class: {110}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
         return(outp);
	}

  if( race_table[ch->race].race_class[iClass]==1)
      {
	strcpy( buf, "\n\r\n\r{060}That combination of class and race is not allowed.\n\r{030}Please choose a class: {110}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
      return(outp);
      }

    case CON_REROLL:
     if( d->connected==CON_REROLL)
      {
        switch ( *argument )
        {
        case 'c': case 'C':
            display_race_selections( d );
	    d->connected = CON_GET_NEW_RACE;
	    break;
          
        case 'y': case 'Y':

        if( ch->vt100 == 0 )
          {
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          }
        else
          write_to_buffer( d, "\033[2J\033[H", 1000000);

	strcpy( buf, "{060}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
          do_help( ch, "motd");
	strcpy( buf, "{110}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
          d->connected = CON_MAIL;
          break;
        default:
          break;
        }
        if( *argument=='y' || *argument=='Y' ||*argument=='c'||*argument=='C')
          break;
      }

case CON_REROLL_AGAIN:
        roll_race(ch);
        if( ch->vt100 == 0 )
          {
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          write_to_buffer( d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
          }
        else
          write_to_buffer( d, "\033[2J\033[H", 1000000);

    sprintf( buf, "{030}Stats for %s the %s %s:{060}\n\r\n\r", ch->name,
        race_table[ch->race].race_name, 
        class_table[ch->class].who_name_long );
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
   sprintf( buf, "{060}Str: {130}%2d    {060}Dex: {130}%2d    {060}Int: {130}%2d    {060}Wis: {130}%2d    {060}Con: {130}%2d\n\r\n\r",
          ch->pcdata->perm_str, ch->pcdata->perm_dex, ch->pcdata->perm_int,
          ch->pcdata->perm_wis, ch->pcdata->perm_con);
        cnt=ch->pcdata->perm_str+ch->pcdata->perm_dex+ch->pcdata->perm_int+
            ch->pcdata->perm_wis+ch->pcdata->perm_con;
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        sprintf( buf , "{060}Average of Stats : {170}%2.1f{060}   The normal average is 13.0\n\r", (float)cnt/5.00);
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        strcpy( buf ,"{060}Languages Known: {030}");
        for(cnt=0;cnt<MAX_RACE;cnt++)
          if(IS_SHIFT(ch->language,cnt))
            {
            strcat(buf, race_table[cnt].race_name);
            strcat(buf," ");
            }
        strcat(buf, "\n\r");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        strcpy(buf, "\n\r{060}You may choose a new race and class by entering 'C'.\n\r");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);
        strcpy(buf, "\n\r{030}Do you wish to keep this character?  (Return rerolls, 'Y' keeps): {110}");
    	write_to_buffer( d, (char *)ansi_translate_text(ch,buf), 1000000);

	d->connected = CON_REROLL;
	break;

   case CON_MAIL:
          STRFREE(d->character->pcdata->mail_address);
          d->character->pcdata->mail_address=str_dup(argument);
	        d->connected = CON_READ_MOTD;
          ch->note_amount = amount_note( ch );

        ch->note_amount = amount_note( ch );
        d->connected = CON_READ_MOTD;
        /* Yes, I want flow through here.  Chaos */

    case CON_READ_MOTD:
      if( !strcasecmp( argument, "term on" ) )
        do_terminal( ch, "on" );

        if(amount_note(ch)>ch->note_amount)
        {
         if(ch->vt100==1)
           write_to_buffer( d, "\033[1;5;7mThere are new notes.\n\r", 0);
         else
           write_to_buffer( d, "There are new notes.\n\r", 0);
        }
        
        if(ch->speak==0)
          {
          roll_race(ch);
          for(cnt=0;cnt<ch->level/10;cnt++)
            add_language(ch);
          write_to_buffer( d, "Race and languages added to char.\n\r", 0);
          }
	add_char( ch );
	d->connected	= CON_PLAYING;
        add_player( ch );
        SET_BIT( ch->act, PLR_COMBINE );
        SET_BIT( ch->act, PLR_AUTO_SPLIT );

  if( ch->recall <1 || room_index[ch->recall]==NULL)
    ch->recall=9755;

  if( ch->pcdata->death_room <1  || room_index[ch->pcdata->death_room]==NULL)
    {
    switch( which_god( ch ) )
      {
      case GOD_CHAOS:
      case GOD_INIT_CHAOS:
             ch->pcdata->death_room = 9719;  break;
      case GOD_ORDER:
      case GOD_INIT_ORDER:
             ch->pcdata->death_room = 9799;  break;
      default:
             ch->pcdata->death_room = ROOM_VNUM_TEMPLE;
      }
    }
  if( ch->pcdata->pvnum == 0)
    {
    Current_pvnum++;
    ch->pcdata->pvnum = Current_pvnum;
    save_sites();
    }
  scan_objects_container( ch );
  /* check_player_hosts( d->host ); */
	if ( ch->level == 0 )
	{
	    OBJ_DATA *obj;

	    ch->level	= 1;
            tst=0;
            for(cnt=0;cnt<MAX_CLASS;cnt++)
              if(ch->mclass[cnt]!=0)
                tst=1;
            if(tst==0)
              ch->mclass[ch->class]=ch->level;
	    ch->exp	= 0;
	    ch->actual_max_hit	= ch->max_hit;
	    ch->actual_max_mana	= ch->max_mana;
	    ch->actual_max_move	= ch->max_move;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    sprintf( buf, "the %s",
              class_table[ch->class].who_name_long);
/* replaced with prev line -dug
		title_table[ch->class][ch->level][ch->sex==SEX_FEMALE?1:0]); */
	    set_title( ch, buf );

/*	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_LIGHT );
*/
	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_BODY );

	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_SHIELD );

            if(class_table[ch->class].weapon!=0)
              {
	      obj = create_object( get_obj_index(class_table[ch->class].weapon),
		                   0 );
	      obj_to_char( obj, ch );
	      equip_char( ch, obj, WEAR_WIELD );
              }
	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
            if( ch->vt100!=0 )
              vt100prompt( ch);
            SET_BIT( ch->pcdata->player2_bits, PLR2_CASTLES);
            SET_BIT( ch->pcdata->player2_bits, PLR2_BATTLE);
	    ch->speed   = get_max_speed (ch);
	    }
	else if ( ch->in_room != NULL )
	{
   	  if ( ch->level<LEVEL_IMMORTAL && room_is_private( ch->in_room ) )
	   {
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	    send_to_char("The room you quit in is now private.\n\r", ch );
	   }
 	  else
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

  /* reset char AC, hitroll, damroll */

        if( ch->in_room->sector_type == SECT_INN )
          {
          int hpgain;
          hpgain = ch->level * (current_time - ch->pcdata->last_time) / 3600;
          if( hpgain < 0 )
            hpgain = 0;
          if( hpgain > 3000 )
            hpgain = 3000;
            
          sprintf( buf, "You have rested for %d points.\n\r", hpgain);
          send_to_char( buf, ch  );
          }
       for (rch=ch->in_room->first_person;rch != NULL;rch=rch->next_in_room)
        if (can_see(rch, ch) && rch!=ch && rch->position>POS_SLEEPING)
         ch_printf(rch, "%s has entered the game.\n\r", get_name(ch));

  if( ch->pcdata->corpse!=NULL)
    obj_to_room( ch->pcdata->corpse, get_room_index( ch->pcdata->corpse_room));

      if( ch->speed > get_max_speed( ch ) )
        ch->speed = get_max_speed( ch );

    if(IS_AFFECTED( ch, AFF_SLEEP))
      {
      ch->position = POS_SLEEPING;
      send_to_char( "You are sleeping.\n\r" , ch);
      break;
      }

	do_look( ch, "auto" );

       if( TEST_GAME)
         send_to_char( "You are entering the test version of MrMud.\n\rWe will take no responsibility for things wrong here.\n\rWe will take no responsibility for problems to your character here.\n\rWe take absolutely no responsibility expect working code on the real game.\n\r", ch);

       if( ch->pcdata->unsafe_password )
         send_to_char( "The system has determined that you have an unsafe password.\n\rPlease change this as soon as possible, using the PASSWORD command.\n\r", ch );

	break;



    }
    return(outp);
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name , bool mobcheck)
{
    /*
     * Reserved words.
     */
    if ( is_name( name, "new chaos emperor order" ) )
	return TRUE;
    if ( is_name( name, "the lord king queen cop police duke duchess count countess sir maiden all auto immortal self someone god supreme demigod ass fuck shit piss crap quit pussy rape whore thief ranger rogue illusionist elementalist necromancer assassin illusionist monk" ) )
	return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;

	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( *pc<'A' || *pc>'z' || (*pc>'Z' && *pc<'a') )
               return FALSE;
	}

    }

    /* Prevent players from naming themselves after mobs. 
       The reason for this is that the $n functions in mob_progs and obj_progs
       are typically stupid and can't decide if it's the player or the mob */
  if( mobcheck ) 
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
              if ( pMobIndex->creator_pvnum==0 )
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}

bool find_ch_game( CHAR_DATA *ch )
{
    CHAR_DATA *prev;

     if( ch == NULL )
       return( FALSE);

	for ( prev = first_char; prev != NULL ; prev = prev->next )
	    if ( prev == ch )
              return( TRUE );

         return( FALSE );
}

bool find_ch_player( CHAR_DATA *ch )
{
    PLAYER_GAME *prev;

     if( ch == NULL )
       return( FALSE);

	for ( prev = first_player; prev != NULL ; prev = prev->next )
	    if ( prev->ch == ch )
              return( TRUE );

         return( FALSE );
}

bool find_ch_room( CHAR_DATA *ch )
{
	CHAR_DATA *prev;

       if( ch==NULL || ch->in_room == NULL)
         return( FALSE );

	for ( prev = ch->in_room->first_person; prev; prev = prev->next_in_room )
	    if ( prev->next_in_room == ch )
		prev->next_in_room = ch->next_in_room;
                  return( TRUE );
      return( FALSE );
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnecting( DESCRIPTOR_DATA *d, CHAR_DATA *ch )
{
  if( d->lookup == FALSE)
    return FALSE;

  d->character = NULL;
  remove_bad_desc( ch , TRUE );

/*   if( ch->desc==NULL)
    {
    d->character=ch;
    ch->desc=d;
    return( TRUE);
    } */

  ch->desc=d;
  d->character=ch;
  d->original=NULL;

  return( TRUE);
}

CHAR_DATA *scan_char( DESCRIPTOR_DATA *d, char *arg )
{
  CHAR_DATA *tch;
  DESCRIPTOR_DATA *td;
  PLAYER_GAME *gpl;

  for( gpl = first_player; gpl!= NULL; gpl=gpl->next)
    if( !strcasecmp( gpl->ch->name, arg))
      return( gpl->ch );

  for( td = first_descriptor; td!= NULL; td=td->next)
    if( td->character!=NULL && td!=d )
    if( !strcasecmp( td->character->name, arg))
      return( td->character );

  for( tch = first_char; tch!= NULL ; tch=tch->next)
    if( !IS_NPC( tch ) && !strcasecmp( tch->name, arg))
      return( tch );

  return(NULL);
}

void do_lookup( CHAR_DATA *ch, char *arg )
{
  CHAR_DATA *tch, *bch;
  DESCRIPTOR_DATA *td;
  PLAYER_GAME *gpl;
  OBJ_DATA *obj;
  char buf[200];
  int cnt;

  if( IS_NPC( ch))
    return;

  if( arg[0]=='\0')
    {
    send_to_char( "SYNTAX: lookup <player name>\n\rThis command scans all forms of lists to locate characters.\n\r", ch);
    return;
    }

  bch = lookup_char( arg );

  for( tch = first_char, cnt=0; tch!= NULL ; tch=tch->next, cnt++)
    if( !IS_NPC( tch ) && !strcasecmp( tch->name, arg))
      {
      sprintf( buf, "%s found in char list, character %d.\n\r", 
          arg, cnt);
      send_to_char( buf, ch );
      }

  for( gpl = first_player, cnt=0; gpl!= NULL; gpl=gpl->next, cnt++)
    if( !strcasecmp( gpl->ch->name, arg))
      {
      sprintf( buf, "%s found in game list, character %d.\n\r", 
          arg, cnt);
      send_to_char( buf, ch );
      }

  for( td = first_descriptor; td!= NULL; td=td->next)
    if( td->character!=NULL)
    if( !strcasecmp( td->character->name, arg))
      {
      sprintf( buf, "%s found in descriptor list (%x). [%d %d]\n\r", 
          arg, (int)td->character, td->descriptor, td->connected);
      send_to_char( buf, ch );
      }

  /* search for objects that have the person for an owner */
  for(obj=first_object;obj!=NULL;obj=obj->next)
   if( obj->carried_by != NULL)
    if((obj->carried_by!=bch)&&!strcasecmp(obj->carried_by->name,arg))
      {
      sprintf( buf, "%s not in game with (%s).\n\r", arg, obj->name);
      send_to_char( buf, ch );
      }

  return;
}

void remove_bad_desc_name( char *name )
{
  CHAR_DATA *fch, *fnext;
  DESCRIPTOR_DATA *dtemp, *dnext, *dt;
  int cnt;
  bool done, broken;

  cnt=0;

  done= FALSE;
  while( ! done )
    {
    broken=FALSE;
    done=TRUE;
    for( dt = first_descriptor; !broken && dt != NULL; dt = dnext )
      {
      dnext = dt->next ;
      if( dt->connected != CON_PLAYING  && dt->connected != -1 &&
          dt->character != NULL && dt->connected != CON_EDITING &&
          dt->character->name != NULL &&
          !strcasecmp( dt->character->name, name ) )
        {
        log_string( "Found a bad descriptor" );
        if( dt->character->desc == dt )
          {
          dt->character->desc = NULL;
          extract_char( dt->character, TRUE );
          }
        dt->character = NULL;
        SET_BIT( dt->comm_flags, COMM_FLAG_DISCONNECT);
        cnt++;
        if( cnt>1)
          more_than_one_close=TRUE;
        broken = TRUE;
        done = FALSE;
        }
      }
    for( fch = first_char; !broken && fch != NULL; fch = fnext )
      {
      fnext = fch->next ;
      if( !IS_NPC( fch )  && 
          !strcasecmp( fch->name, name ) &&
          ( fch->desc != NULL && 
           (fch->desc->connected != CON_PLAYING && 
            fch->desc->connected != CON_EDITING)))
        {
        dtemp = fch->desc;
        log_string( "Found a bad char" );
        extract_char( fch, TRUE );
        if( dtemp != NULL )
          {
          dtemp->character = NULL;
          SET_BIT( dtemp->comm_flags, COMM_FLAG_DISCONNECT);
          }
        cnt++;
        if( cnt>1)
          more_than_one_close=TRUE;
        broken = TRUE;
        done = FALSE;
        }
      }
    }
 return;
}

void remove_bad_desc( CHAR_DATA *ch , bool Force)
{
  DESCRIPTOR_DATA *dtemp,*dnext;
  int cnt;

  cnt=0;


    for( dtemp=first_descriptor; dtemp!=NULL ; dtemp=dnext)
      {
      dnext=dtemp->next;
      if( (dtemp->character==ch || dtemp->original==ch) && 
           ( Force || (dtemp->connected != CON_PLAYING 
                    && dtemp->connected != CON_EDITING)))
        {
        if( dtemp->original==ch)
          {                              
        if(IS_NPC(dtemp->character) && dtemp->character->pIndexData->vnum==9900)
            SET_BIT( dtemp->character->act, ACT_WILL_DIE);
          dtemp->character->desc = NULL;
          dtemp->character       = dtemp->original;
          dtemp->original        = NULL;
          }
        SET_BIT( dtemp->comm_flags, COMM_FLAG_DISCONNECT);
        if( dtemp->character->desc == dtemp )
          dtemp->character->desc = NULL;
        dtemp->character = NULL;
        cnt++;
        if( cnt>1)
          more_than_one_close=TRUE;
        }
      }
 return;
}

void stop_idling( CHAR_DATA *ch )
{
    CHAR_DATA *rch;
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   (ch->desc->connected != CON_PLAYING
    &&   ch->desc->connected != CON_EDITING)
    ||   ch->desc->character != ch
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );

    if ( ch->level<LEVEL_IMMORTAL && room_is_private( ch->was_in_room ) )
     {
      char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
      send_to_char("The room you were idling in is now private.\n\r", ch );
     }
    else
      char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
 
    /* Another hole in stealth/invis Martin 6/8/98 */
    for (rch=ch->in_room->first_person;rch != NULL;rch=rch->next_in_room)
     if (can_see(rch, ch) && rch!=ch && rch->position>POS_SLEEPING)
      ch_printf(rch, "%s has returned from the void.\n\r",
       get_name(ch));

    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if( !ALLOW_OUTPUT )
      return;

    if ( txt != NULL && ch!=NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, (char *)txt, 0 /* strlen(txt) */ );
    return;
}

void send_to_char_color( char *txt, CHAR_DATA *ch )
{
    if( !ALLOW_OUTPUT )
      return;

    if ( txt != NULL && ch!=NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, 
        (char *)ansi_translate_text(ch,txt), 0 /* strlen(txt) */ );
    return;
}

/*
 * Write to one char.  With string length for speed.
 */
void ship_to_char( const char *txt, CHAR_DATA *ch , int leng)
{
    if ( txt != NULL && ch!=NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, (char *)txt, leng );
    return;
}

/*
 * Write to one char.  With string length for speed.
 */
void ship_to_char_color( char *txt, CHAR_DATA *ch , int leng)
{
    if ( txt != NULL && ch!=NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, (char *)ansi_translate_text(ch, txt), leng );
    return;
}

#define NUM_FAKE_NAME 15
/*
 * The primary output interface for formatted output.
 */
void act( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
    static char * const he_she	[] = { "it",  "he",  "she" };
    static char * const him_her	[] = { "it",  "him", "her" };
    static char * const his_her	[] = { "its", "his", "her" };
    static char * const fake_name [] = {"Garfield","Jim Morrison",
      "Hillary","A house plant","Someone's forehead","Your mother",
      "Homer Simpson","Chaos","Order","Demise","A lollipop",
      "Ghandi","Your index finger","A large worm","The ground"};

    char buf[MAX_STRING_LENGTH];
    char buf2[250];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

  if( !ALLOW_OUTPUT )
      return;

    /* added check for null ch -Dug 12/7/93 */
    if ( ch == NULL )
    {
        bug( "Act: null ch.", 0 );
        return;

    }
    /* added check for null ch -Presto 3/28/99
    if ( ch->in_room == NULL )
    {
        bug( "Act: null ch->in_room.", 0 );
        return;
    } */

    to = ch->in_room->first_person;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}
        if( vch->in_room == NULL )
    {
        bug( "Act: null victim room.", 0 );
        return;
    }
	to = vch->in_room->first_person;
    }
    
    for ( ; to != NULL; to = to->next_in_room )
    {
        if ((!to->desc
        && (  IS_NPC(to) && !IS_SET(to->pIndexData->progtypes, ACT_PROG) ))
        ||   !IS_AWAKE(to) )
            continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

/* poll the block list of the victim */
        if(blocking(to,ch))
          continue;

	point	= buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
            /*sprintf( buf2, "Act: %s missing arg2 for code %d.", ch->name, *str);*/
            sprintf( buf2, "Act: missing arg2 for code %d.", *str);
               log_string( buf2 );
		i = " <@@@> ";
	    }
	    else
	    {
            if(!IS_AFFECTED(to,AFF2_HALLUCINATE))
              switch ( *str )
		{
		default: 
                  /*sprintf( buf2, "Act: %s bad code %d.", ch->name, *str);*/
                  sprintf( buf2, "Act: act bad code %d.", *str);
                  log_string( buf2 );
		  i = " <@@@> ";					break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;				break;
		case 'T': i = (char *) arg2;          			break;
             /* case 'n': i = PERS( ch,  to  );				break;
		case 'N': i = PERS( vch, to  );				break;*/
		case 'n': i = get_name( ch );				break;
		case 'N': i = get_name( vch );				break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
            else /* THIS IS THE HALLUCINATE VERSION...DON'T PANIC -ORDER */
              {
              
              switch ( *str )
		{
		default: 
                  sprintf( buf2, "Act: act bad code %d.", *str);
                  log_string( buf2 );
		  i = " <@@@> ";
                  break;
                case 't': case 'T': case 'n': case 'N':
                case 'p': case 'P': case 'd':
                  i=fake_name[number_range(0,NUM_FAKE_NAME-1)];
                  break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;
		}
              }
	    }
		
        if( i == NULL )
          i = "nothing";
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}
	*point++ = '\n';
	*point++ = '\r';
	*point++ = '\0';
	buf[0]   = UPPER(buf[0]);
        if(to->desc)
          write_to_buffer(to->desc,((char *)ansi_translate_text(to,buf)),point-buf);
        if /*( MOBtrigger && */
           ((  IS_NPC(to) && IS_SET(to->pIndexData->progtypes, ACT_PROG)
                && !IS_AFFECTED(to, AFF2_POSSESS) )
        &&   IS_AWAKE(to)) 
          mprog_act_trigger(buf,to,ch,obj1,vch);
    }
    MOBtrigger=TRUE;
    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

void force_help( DESCRIPTOR_DATA *d, char *argument)
{
  HELP_DATA *pHelp;

  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	  if ( is_name( argument, pHelp->keyword ))
	    {
	    if ( pHelp->text[0] == '.' )
		    write_to_descriptor( d, pHelp->text+1,0);
	    else
		    write_to_descriptor( d, pHelp->text,0);
	    return;
	    }
    }
return;
}

bool is_desc_valid( CHAR_DATA *ch)
{
   /* DESCRIPTOR_DATA *d;  */
  /* PLAYER_GAME *gpl; */
/*  bool good_desc , good_gpl; */
  
  /* good_desc = FALSE; 
  for ( d = first_descriptor; !good_desc && d != NULL; d = d->next )
    if( d==ch->desc && d->original == NULL || d->original == ch ) 
      good_desc = TRUE;

  if( good_desc )
    return( TRUE );
  return( FALSE );  */

  if( ch == NULL )
    return( FALSE );

  if( ch->desc != NULL && ch->desc->character == ch )
    return( TRUE );
  else
    return( FALSE );  
}


void add_player( CHAR_DATA *ch )
{
  PLAYER_GAME *gpl, *fpl, *lpl;

  if( IS_NPC( ch ) )
    return;
 
  CREATE( gpl, PLAYER_GAME, 1);
  total_characters++;

  if( first_player==NULL)
    {
    gpl->ch=ch;
    gpl->next=NULL;
    first_player=gpl;
    return;
    }
  lpl=first_player;
  for( fpl=first_player; fpl!=NULL; fpl=fpl->next)
    {
    if(ch->level < fpl->ch->level ||
       ( ch->level==fpl->ch->level && ch->played <= fpl->ch->played ) )
          /* Add ranking inside of all lists of players  - Chaos 2/19/95 */
      {
      gpl->ch=fpl->ch;
      gpl->next=fpl->next;
      fpl->ch=ch;
      fpl->next=gpl;
      return;
      }
    lpl=fpl;
    }
  lpl->next=gpl;
  gpl->ch=ch;
  gpl->next=NULL;
  return;
}

void sub_player( CHAR_DATA *ch )
{
  PLAYER_GAME *fpl, *lpl;
  lpl=first_player;
  for( fpl=first_player; fpl!=NULL; fpl=fpl->next)
    {
    if( fpl->ch==ch)
      {
      if(fpl==first_player)
        {
        first_player=fpl->next;
        DISPOSE( fpl );
        total_characters--;
        return;
        }
      lpl->next=fpl->next;
      DISPOSE( fpl );
      total_characters--;
      return;
      }
    lpl=fpl;
    }
  return;
}
CHAR_DATA * start_partial_load( CHAR_DATA *ch, char *argument)
{
  bool exists, loaded=FALSE;
  CHAR_DATA *fch;
  static DESCRIPTOR_DATA d_zero;
  int expire;
  char buf[181];
  DESCRIPTOR_DATA *d;

  if ( !check_parse_name( argument, FALSE ) )
  {
   send_to_char( "That character does not exist.\n\r", ch );
   return NULL;
  }
  d = NULL;
  fch=lookup_char( argument );

  if( fch==NULL)
  {
   CREATE(d, DESCRIPTOR_DATA, 1);
   *d	= d_zero;
   d->original=NULL;
   d->descriptor = -999;  /* Special case for partial loads */
   load_error = FALSE;
   exists = load_char_obj( d, argument );
   fch   = d->character;
   fch->desc = d;
   loaded=TRUE;
  }
  else
  {
   if( is_desc_valid( fch ) )
    d = fch->desc;
   exists=TRUE; 
  }

  if( (expire = character_expiration( fch )) < 0 )
   exists = FALSE;

  if( !exists )
  {
   sprintf( buf, "The character named '%s' cannot be found.\n\r", argument);
   send_to_char( buf, ch);
   if( d != NULL )
   {
    d->character=NULL;
    d->original=NULL;
   }
   if( fch != NULL )
   {
    fch->desc=NULL;
    extract_char( fch, TRUE);
   }
   if(loaded && d != NULL )
    DISPOSE( d );
   return NULL;
  }
  return fch;
}
void clear_partial_load( CHAR_DATA *ch )
{
 DESCRIPTOR_DATA * d = ch->desc;

 ch->desc=NULL;
 extract_char( ch, TRUE);
 d->character=NULL;
 d->original=NULL;
 DISPOSE( d );
}

void do_finger( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *fch;
  char const *class;
  bool loaded = FALSE;
  char buf[181], buf2[MAX_STRING_LENGTH], buf3[MAX_INPUT_LENGTH];
  char outbuf[MAX_STRING_LENGTH];
  char buf_name[80], buf_lang[80], buf_race[80];
  char *pt;
  int length=0;
  int expire;
  int time_left,cnt;
  sh_int rel_days;
  sh_int rel_hrs;
  sh_int rel_mins;


  fch=lookup_char( argument );

  if( fch==NULL)
  {
   fch = start_partial_load( ch, argument);
   if (fch == NULL)
    return;
   loaded = TRUE;
  }

  if( ch->vt100==0)
    length = sprintf( outbuf, "  Information on %s%s\n\r", get_name( fch ),
           fch->pcdata->title);
  else
    length = sprintf( outbuf, "\033[1m  Information on %s%s:\n\r", get_name( fch ),
           fch->pcdata->title);
  FINGER_MODE=TRUE;
    if ( fch->description[0] != '\0' )
    {
        length = str_apd_max(outbuf,  (const char *)ansi_translate_text( ch,
            fch->description),length, MAX_STRING_LENGTH);
        length = str_apd_max (outbuf, "\n\r", length, MAX_STRING_LENGTH);
    }
    else
    {
        str_apd_max(outbuf, "You see nothing special.\n\r", length, MAX_STRING_LENGTH);
    }
      
        if (ch->ansi !=0 && ch->pcdata->color[0]!=0)
            sprintf(buf2, "\033[0;%d;%dm", ch->pcdata->color[0][0],
		ch->pcdata->color[1][0]);
        else buf2[0]='\0';

        length = str_apd_max(outbuf, buf2, length, MAX_STRING_LENGTH);
	/*
	 * Figure out what to print for class.
	 */
	class = class_table[fch->class].who_name;
	switch ( fch->level )
	{
	default: break;
	case MAX_LEVEL - 0: class = "GOD"; break;
	case MAX_LEVEL - 1: 
                 if( fch->sex==SEX_FEMALE )
                   class = "DUC"; 
                 else
                   class = "DUK"; 
                 break;
        case MAX_LEVEL - 2: if (which_god(fch) == GOD_POLICE)
                             class = "COP";
                            else
                             class = "COU"; break;
	/*case MAX_LEVEL - 3: class = "ANG"; break;*/
	}

        if( IS_IMMORTAL(fch))
          strcpy( buf_race, "---" );
        else
          strcpy(buf_race,race_table[fch->race].race_name);
        buf_race[3]='\0';

        /* get rid of stinking ESC sequence for nonVT100 -Dug 12/2/93 */
        buf_lang[0]='\0';

    sprintf( buf_lang,"Ill:%2d  Ele:%2d  Rog:%2d  Ran:%2d  Nec:%2d  Mon:%2d  Asn:%2d",
          fch->mclass[0],
          fch->mclass[1],
          fch->mclass[2],
          fch->mclass[3],
          fch->mclass[4],
          fch->mclass[5],
          fch->mclass[6]);
	/*
	 * Format it up.
	 */
  strcpy(buf_name, fch->name);
  buf_name[11]='\0';
  while(strlen(buf_name)<11)
    strcat ( buf_name, " ");
	sprintf( buf2, "%s%s%s [%2d %s %s] %s%s%s\n\r",
          ch->vt100==1 ? "\033[1m" : "" ,
          capitalize(buf_name),
          ch->vt100==1 ? "\033[m" : "" ,
	        fch->level,
	        class,
          buf_race,
          ch->vt100==1 ? "\033[1m"  : "" ,
          buf_lang,
          ch->vt100==1 ? "\033[m" : "" );

  length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);
  if (ch->ansi)
  if (ch->pcdata->color[0]!=0)
     sprintf(buf2, "\033[0;%d;%dm", ch->pcdata->color[0][0],
	ch->pcdata->color[1][0]);
   length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);

    sprintf( buf2, "Age: %d.  Played: %d hours.\n\r",
	get_age( fch ), (int) fch->played/3600+fch->pcdata->previous_hours );
  length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);

  if( fch->pcdata->reincarnation > 0 )
    {
    sprintf( buf2, "%s has been reincarnated %d times.\n\r", 
               capitalize(fch->name), fch->pcdata->reincarnation );
    length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);
    }
  expire = character_expiration(fch);
  if( ch->level == MAX_LEVEL || (expire < 60 ) )
    {
    sprintf( buf2, "This player file will expire in %d days.\n\r", expire );
    length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);
    }

   if( which_god( fch ) != GOD_NEUTRAL)
   {
     switch( which_god( fch ) )
       {
       case GOD_INIT_ORDER: 
         sprintf(buf,  "This player is an initiate of Order.\n\r");break;
       case GOD_INIT_CHAOS: 
         sprintf(buf,  "This player is an initiate of Chaos.\n\r");break;
       case GOD_ORDER: 
         sprintf(buf,  "This player follows Order.\n\r");break;
       case GOD_CHAOS: 
         sprintf(buf,  "This player follows Chaos.\n\r");break;
       case GOD_DEMISE:
    sprintf(buf,  "This unfortunate wretch is a minion of Demise.\n\r");break;
       case GOD_POLICE: 
    sprintf(buf,  "This player is an Enforcer of the Realms.\n\r");break;
       case GOD_CREATOR:
    sprintf(buf,  "This player is an Area Creator.\n\r");break;
       }
   length = str_apd_max (outbuf, buf, length, MAX_STRING_LENGTH);
   }
   if( IS_SET( fch->act, PLR_OUTCAST) )
    {
     sprintf(buf, "%s is an Outcast!\n\r", get_name(fch));
     length = str_apd_max( outbuf, buf, length, MAX_STRING_LENGTH);
    }
    else if ( fch->pcdata->clan)
   {
    if ( !strcasecmp( fch->name, fch->pcdata->clan->leader  ) )
      sprintf(buf, "%s is the founder of %s.\n\r", get_name(fch), fch->pcdata->clan_name);
    else
    if ( !strcasecmp( fch->name, fch->pcdata->clan->number1 )
    ||   !strcasecmp( fch->name, fch->pcdata->clan->number2 )
    ||   !strcasecmp( fch->name, fch->pcdata->clan->number3 )
    ||   !strcasecmp( fch->name, fch->pcdata->clan->number4 ) )
    
      sprintf(buf, "%s is a leader of %s.\n\r", get_name(fch), fch->pcdata->clan_name);
    else
      sprintf(buf, "%s is a member of %s.\n\r", get_name(fch), fch->pcdata->clan_name);
    length = str_apd_max( outbuf, buf, length, MAX_STRING_LENGTH);
   }
 
   if( fch->pcdata->arrested == TRUE )
     {
       time_left = fch->pcdata->jailtime -
                   (current_time - fch->pcdata->jaildate);
       if(time_left > 0)
         {
           rel_days   = time_left / 86400;
           time_left -= rel_days * 86400;
           rel_hrs    = time_left / 3600;
           time_left -= rel_hrs * 3600;
           rel_mins   = time_left / 60;
           if(rel_mins == 0)
	     rel_mins = 1;
           sprintf(buf2, "Imprisoned in the dungeons of the realms for %d days, %d hours, %d minutes.\n\r", rel_days, rel_hrs, rel_mins);
           length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);
         }
     }   

   if( which_god( fch )==GOD_CHAOS || which_god(fch)==GOD_ORDER ||
       which_god( fch )==GOD_DEMISE || which_god(fch)==GOD_POLICE )
    sprintf( buf2, "%s's standings:\n\rNeutrals Dead: %3d    Neutrals that have killed %s: %3d\n\rEnemies Dead:  %3d    Enemies that have killed %s:  %3d\n\r                      Monsters that have killed %s: %3d\n\r",
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_PC],
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_BY_PC],
        fch->pcdata->history[HISTORY_KILL_EN],
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_BY_EN],
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_BY_NPC]);
    else
    sprintf( buf2, "%s's standings:\n\rPlayers Dead: %3d    Players that have killed %s:  %3d\n\r                     Monsters that have killed %s: %3d\n\r",
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_PC],
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_BY_PC],
        capitalize(fch->name),
        fch->pcdata->history[HISTORY_KILL_BY_NPC]);
  length = str_apd_max (outbuf,  buf2, length, MAX_STRING_LENGTH);

  if( fch->pcdata->army_status > 0 )
    {
    if( fch->pcdata->army_status == 1 )
      sprintf( buf2, "Army Status: Active      Kills: %3d    Losses: %3d\n\r",
        fch->pcdata->history[HISTORY_KILL_ARMY],
        fch->pcdata->history[HISTORY_LOSE_ARMY]);
    else
      sprintf( buf2, "Army Status: Retired     Kills: %3d    Losses: %3d\n\r",
        fch->pcdata->history[HISTORY_KILL_ARMY],
        fch->pcdata->history[HISTORY_LOSE_ARMY]);
    length = str_apd_max ( outbuf, buf2, length, MAX_STRING_LENGTH);
    }
   if (!IS_SET(ch->pcdata->player2_bits, PLR2_VICTIM_LIST))
   {
     if (fch->pcdata->killname[ch->pcdata->killnum][0] !='\0')
 
      length = str_apd_max ( outbuf, "Victim list:\n\r", length, MAX_STRING_LENGTH);
    for (cnt=fch->pcdata->killnum;cnt<MAX_KILL_TRACK;cnt++)
    {
     if (fch->pcdata->killname[cnt][0]=='\0') continue;
     pt=fch->pcdata->killname[cnt]+1;
     sprintf(buf, "%s\n\r", pt);
     length = str_apd_max (outbuf, buf, length, MAX_STRING_LENGTH);
    }
    for (cnt=0;cnt<fch->pcdata->killnum;cnt++)
    {
     if (fch->pcdata->killname[cnt][0]=='\0') continue;
     pt=fch->pcdata->killname[cnt]+1;
     sprintf(buf, "%s\n\r", pt);
     length = str_apd_max (outbuf, buf, length, MAX_STRING_LENGTH);
    }
   }
  if (IS_IMMORTAL(ch) || ch->which_god == GOD_POLICE)
    {
    sprintf( buf2, "Internet Email address: %s\n\r", fch->pcdata->mail_address);
    length = str_apd_max ( outbuf, buf2, length, MAX_STRING_LENGTH);
    }

  if( *fch->pcdata->html_address != '\0' )
    {
    strcpy( buf3, fch->pcdata->html_address );
    for( pt=buf3; *pt != '\0'; pt++)
      if( *pt == '*' )
        *pt = '~';
    sprintf( buf2, "Internet Home Page: %s\n\r", buf3);
    length = str_apd_max (outbuf,  buf2, length, MAX_STRING_LENGTH);
    }

  /* output castle info */
  if(fch->pcdata->castle!=NULL)
    {
    sprintf(buf2,"Has a castle that cost $%d000 to build.\n\r",
            fch->pcdata->castle->cost);
    length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);
    }

  if( ch->level>96 || which_god(ch)==GOD_POLICE )
    {
    if( fch->pcdata->give_prac_pos > 0)
      {
      sprintf( buf2, "Total practices given: %d\n\r", 
          fch->pcdata->give_prac_pos );
      length = str_apd_max (outbuf, buf2, length, MAX_STRING_LENGTH);
      }
    if( fch->pcdata->give_prac_neg > 0)
      {
      sprintf( buf2, "Total practices taken: %d\n\r", 
          fch->pcdata->give_prac_neg );
      length = str_apd_max ( outbuf, buf2, length, MAX_STRING_LENGTH);
      }

    get_attack_string( fch, ch, buf2 );
    length = str_apd_max ( outbuf, buf2, length, MAX_STRING_LENGTH);


    }

  if( loaded || IS_SET(fch->act,PLR_WIZINVIS)
      || (is_affected(fch, gsn_greater_stealth) && !(IS_SET(ch->act, PLR_HOLYLIGHT) || (is_affected(ch, gsn_truesight) && ch->mclass[CLASS_ILLUSIONIST] >= fch->level) )))
    {
    if(fch->pcdata->last_time>0)
      {
      sprintf( buf2, "Last time in the realm: %s\r", 
	  ctime((const time_t *)&fch->pcdata->last_time));
      length = str_apd_max (outbuf,  buf2, length, MAX_STRING_LENGTH);
      }
    if(loaded)
     {
      if (IS_IMMORTAL(ch) || ch->which_god == GOD_POLICE)
        sprintf(buf2,"Internet computer last on: %s\n\r", fch->desc->old_host);
      else
        sprintf(buf2,"Internet domain last on: %s\n\r", fch->desc->old_domain);
     }
    else
      if( is_desc_valid( fch))
      {
      if (IS_IMMORTAL(ch) || ch->which_god == GOD_POLICE)
       {
        if (fch->desc->old_host != NULL && *fch->desc->old_host!='\0')
        sprintf(buf2,"Internet computer last on: %s\n\r", fch->desc->old_host);
       }
      else
       {
        if (fch->desc->old_domain != NULL && *fch->desc->old_domain!='\0')
        sprintf(buf2,"Internet domain last on: %s\n\r", fch->desc->old_domain);
       }
      }
    length = str_apd_max (outbuf,  buf2, length, MAX_STRING_LENGTH);
    if(!loaded)
      {
      FINGER_MODE=FALSE;
      send_to_char(outbuf, ch);
      return;
      }
    }
  else
   if( is_desc_valid( fch))
    {
    if (IS_IMMORTAL(ch) || ch->which_god == GOD_POLICE)
     sprintf(buf2,"Internet computer on: %s\n\r", fch->desc->host);
    else
     sprintf(buf2,"Internet domain on: %s\n\r", fch->desc->domain);
    length = str_apd_max (outbuf,  buf2, length, MAX_STRING_LENGTH);
    }
  ship_to_char(ansi_translate_text(ch,outbuf), ch, 0);
  if (loaded)
   clear_partial_load (fch);   
  FINGER_MODE=FALSE;
  return;
}


void do_create( CHAR_DATA *ch, char *arg)
{
  DESCRIPTOR_DATA *d;
  char arg1[100], arg2[100], arg3[100];

  return;

  arg = one_argument( arg, arg1);
  arg = one_argument( arg, arg2);
  arg = one_argument( arg, arg3);


  if( IS_NPC( ch))
    return;
  if( !is_desc_valid( ch) ||
    ( ch->pcdata->creation_room == 0 && ch->level< 99 ) )
    {
    send_to_char( "You cannot do that.\n\r", ch);
    return;
    }
  if( *arg1 != '\0')    /* God picked a player */
    ch = lookup_char( arg1 );

  d=ch->desc;
  if( ch->pcdata->creation_room==0)
    ch->pcdata->creation_room=ch->in_room->vnum;

  d->connected = CREATOR_MAIN;
  d->creator_location = ch->pcdata->creation_room;
  
  if( ch->pcdata->creator_zone == 0)
    {
    /* determine new zone number */
    }

if( ch->vt100 != 0)
   vt100off( ch);

  extract_char( ch, TRUE);

  d->character=NULL;
  d->creator_zone = ch->pcdata->creator_zone;

  display_creator( d ); /* Output first menu */
  return;
}

  
bool creator( DESCRIPTOR_DATA *d, char *argument)
  {
  char key;
  ROOM_INDEX_DATA *room;
  int door;

  room = room_index[ d->creator_location ];

  key=argument[0];
  if( key>='A' && key<='Z')
    key+=('a'-'A');
  
  switch( d->connected )
    {
  case CREATOR_MAIN:
    switch( key )
      {
      case 'e': d->connected=CREATOR_EDIT_ROOM;		 break;
      case 'n': d->connected=CREATOR_NEW_ROOM; break;
      case 'm': d->connected=CREATOR_MOVE; break;
      case 'x':
        /* do save here */
        write_to_buffer( d, "Thank you very much.\n\r", 1000000);
        close_socket( d, TRUE);
        return( TRUE );
      }

    break;

case CREATOR_NEW_ROOM:
    if( argument[0]=='-')
      {
      d->connected = CREATOR_MAIN;
      break;
      }
    write_to_buffer( d, "That is not an option.\n\r", 1000000);
    break;
    

case CREATOR_EDIT_ROOM:
    switch( argument[0] )
      {
      case '-':
        d->connected = CREATOR_MAIN;	 break;
      case 'n':
        d->connected = CREATOR_EDIT_ROOM_NAME;	 break;
      case 'd':
        d->connected = CREATOR_EDIT_ROOM_DESC;	 break;
      case 'e':
        d->connected = CREATOR_EDIT_ROOM_EXTRAS;	 break;
      case 'f':
        d->connected = CREATOR_EDIT_ROOM_FLAGS;	 break;
      case 's':
        d->connected = CREATOR_EDIT_ROOM_SECTOR;	 break;
      }
    break;

case CREATOR_EDIT_ROOM_SECTOR:
    switch( key )
      {
      case '0':  room->sector_type = 0; break;
      case '1':  room->sector_type = 1; break;
      case '2':  room->sector_type = 2; break;
      case '3':  room->sector_type = 3; break;
      case '4':  room->sector_type = 4; break;
      case '5':  room->sector_type = 5; break;
      case '6':  room->sector_type = 6; break;
      case '7':  room->sector_type = 7; break;
      }
    d->connected = CREATOR_EDIT_ROOM;
    break;
  
case CREATOR_MOVE:
    door=-1;
    switch( key )
      {
      case '-': d->connected = CREATOR_MAIN; break;
      case 'n': door=0; break;
      case 'e': door=1; break;
      case 's': door=2; break;
      case 'w': door=3; break;
      case 'u': door=4; break;
      case 'd': door=5; break;
      }
    if( door>=0 )
      {
      if( room->exit[door] == NULL ||
          room->exit[door]->to_room == NULL)
        {
        write_to_buffer( d, "You can't move that direction.\n\r", 1000000);
        return( FALSE );
        }
      d->creator_location = room->exit[door]->to_room->vnum;
      d->connected = CREATOR_MAIN;
      return( FALSE );
      }
    write_to_buffer( d, "That is not an option.\n\r", 1000000);
    return( FALSE );

  default:
      d->connected = CREATOR_MAIN;
    }
  return( FALSE);
  }

void display_creator( DESCRIPTOR_DATA *d )
{
    ROOM_INDEX_DATA *room;
    EXIT_DATA *pexit;
    int door, cnt;
    char buf[250], buf2[250];

   room = room_index[ d->creator_location ];

  switch( d->connected )
    {
  case CREATOR_MAIN:
      write_to_buffer( d, "Main Menu for Area Creation.\n\r(N) New Room\n\r(E) Edit Room\n\r(M) Move\n\r(X) Exit and Save\n\r", 1000000);
      sprintf( buf, "Current room #%d.\n\r", d->creator_location);
      write_to_buffer( d, buf, 1000000);
      break;

  case CREATOR_MOVE:
      write_to_buffer( d, "(-) Previous Menu\n\r", 1000000);
      sprintf( buf, "Moving current room #%d.\n\r", d->creator_location);
      write_to_buffer( d, buf, 1000000);

    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = room->exit[door] ) != NULL
	    && pexit->to_room != NULL
	    && can_use_exit(d->character, room, door) )
	    {
              strcpy(buf, dir_name[door]);
              while(strlen(buf)<7)
                strcat(buf,"-");

              if(buf[0]>'Z')
                buf[0]-=('a'-'A');
             strcat(buf, pexit->to_room->name);
             strcat( buf, "\n\r");
           write_to_buffer( d, buf, 1000000);
	 }
    }
      
      break;
     
case CREATOR_NEW_ROOM:
      write_to_buffer( d, "(-) Previous Menu\n\r", 1000000);
      sprintf( buf, "New room #%d.\n\r", d->creator_location);
      write_to_buffer( d, buf, 1000000);
      break;

case CREATOR_EDIT_ROOM:
      write_to_buffer( d, "(-) Previous Menu\n\r", 1000000);
      sprintf( buf, "Room #%u\n\r(N) Name: %s<\n\r(D) Description:\n\r%s\n\r",
          room->vnum, room->name, room->description);
      write_to_buffer( d, buf, 1000000);
      sprintf( buf, "(F) Flags: %s\n\r(E) Extra Keywords: %s\n\r(S) Sector Type: %s\n\r(-) Back\n\r",
          "<flag list>", "<keyword list>", sector_name[room->sector_type]);
      write_to_buffer( d, buf, 1000000);
      break;
case CREATOR_EDIT_ROOM_SECTOR:
      strcpy( buf, "Choose new type:\n\r" );
      for(cnt=0; cnt<8; cnt++)
        {
        sprintf( buf2, "(%d) %s\n\r", cnt, sector_name[cnt]);
        strcat( buf, buf2);
        } 
      strcat( buf, "(-) Back\n\r");
      write_to_buffer( d, buf, 1000000);
      break;
      
  }
 return;
}

int scan_container( CHAR_DATA *ch, OBJ_DATA *cobj )
{
  OBJ_DATA *obj;
  int drop, cnt;

  drop = count_total_objects( cobj ) - MAX_OBJECTS_IN_CONTAINER;
  if( drop>0 )
    for( cnt=0; cnt<drop; cnt++)
      {
      obj=cobj->first_content;
      obj_from_obj( obj );
      obj_to_room( obj, ch->in_room );
      }
  else
    drop=0;
 
    for( obj=cobj->first_content ; obj!=NULL; obj=obj->next_content )
      if( obj->item_type == ITEM_CONTAINER)
        drop+=scan_container( ch, obj );
  return( drop );
}

void scan_objects_container( CHAR_DATA *ch )
{
  OBJ_DATA *obj;
  int drop;

  drop=0;
    for( obj=ch->first_carrying ; obj!=NULL; obj=obj->next_content )
      if( obj->item_type == ITEM_CONTAINER)
        drop+=scan_container( ch, obj );
  if( drop!= 0)
     {
     send_to_char( "You dropped some stuff.\n\r", ch);
     save_char_obj(ch, NORMAL_SAVE);
     }
  return;
}

bool is_valid_password( char *pass )
{
  char *pt;
  bool found_number;
  bool good_char;


  for( found_number=FALSE, pt=pass; *pt!='\0'; pt++)
    {
    good_char = FALSE;
    if( *pt >= 'a' && *pt <= 'z' )
      good_char = TRUE;
    if( *pt >= 'A' && *pt <= 'Z' )
      good_char = TRUE;
    if( *pt >= '0' && *pt <= '9' )
      {
      good_char = TRUE;
      found_number = TRUE;
      }
    if( !good_char )
      return( FALSE );
    }
  if( !found_number )
    return( FALSE );
  else
    return( TRUE );
}

void do_mpquiet( CHAR_DATA *ch, char *arg )
{
  if( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) )
    return;

  if( !strcasecmp( arg, "on" ))
    ALLOW_OUTPUT = FALSE;

  if( !strcasecmp( arg, "off" ))
    ALLOW_OUTPUT = TRUE;

  return;
}
  

int count_links( HOST_NAMES *host )
{
  int cnt;
  DESCRIPTOR_DATA *dtemp;

  for(cnt=0, dtemp=first_descriptor; dtemp!=NULL; dtemp=dtemp->next )
    if( host == dtemp->host_index )
      cnt++;

  return( cnt );
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_INPUT_LENGTH];
  char tbuf[MAX_INPUT_LENGTH];
  int blen;

  if( IS_NPC( ch ) )
    return;

  smash_tilde( argument );
  if( *argument == ' ')
    argument ++;
  str_cpy_max( buf, argument, 40 );
  blen = strlen( buf );
  while( *buf != '\0'  && *(buf + blen - 1) == ' ')
    {
    *(buf+blen-1)='\0';
    blen--;
    }
  if( blen > 0 )
    {
    STRFREE (ch->pcdata->prompt_layout );
    if( *buf == '$' && *(buf +1)=='\0')
      *buf = '\0';
    ch->pcdata->prompt_layout = str_dup( buf );
    if( *buf == '\0' )
      sprintf( tbuf, "Prompt layout set to default.\n\r" );
    else
      sprintf( tbuf, "Prompt layout set to <%s>\n\r", buf );
    send_to_char( tbuf, ch );
    return;
    }
  
  if( *ch->pcdata->prompt_layout == '\0' )
    sprintf( tbuf, "Prompt layout set to default.\n\r" );
  else
    sprintf( tbuf, "Prompt layout set as <%s>\n\r", ch->pcdata->prompt_layout );
  send_to_char( tbuf, ch );
  return;
}

char *prompt_return( CHAR_DATA *ch, char *layout )
{
  char tbuf[MAX_INPUT_LENGTH];
  char *pti, *pto;
  bool last_was_str;

  if( layout == NULL || *layout == '\0' )
    sprintf( prompt_buffer,  "<%dhp %dm %dmv> ", ch->hit, ch->mana, ch->move );
  else
    {
    last_was_str = FALSE;
    pti = layout;
    pto = prompt_buffer;
    *pto = '<';
    pto++;
    while( *pti != '\0' )
      {
      if( last_was_str )
        switch( *pti )
          {
          case 'h':
            sprintf( tbuf, "%d", ch->hit );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'H':
            sprintf( tbuf, "%d", ch->max_hit );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'v':
            sprintf( tbuf, "%d", ch->move );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'V':
            sprintf( tbuf, "%d", ch->max_move );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'm':
            sprintf( tbuf, "%d", ch->mana );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'M':
            sprintf( tbuf, "%d", ch->max_mana );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'x':
            sprintf( tbuf, "%d", ch->exp );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case 'g':
            sprintf( tbuf, "%d", ch->gold );
            strcpy( pto, tbuf );
            pto += strlen( tbuf );
            last_was_str = FALSE;
            break;

          case '$':
            *pto = '$';
            last_was_str = FALSE;
            pto++;
            break;
          
          default:
            *pto = '$';
            pto ++;
            *pto = *pti;
            pto ++;
            last_was_str = FALSE;
            break;
          }
      else
        {
        if( *pti != '$' )
          {
          *pto = *pti;
          pto++;
          }
        else
          last_was_str = TRUE;
        }
      pti ++;
      }
    *pto = '>';
    pto++;
    *pto = '\0';
    }
  return( (char *) ansi_translate_text(ch, prompt_buffer) );
  /*return( prompt_buffer );*/
}

void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];      /* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_combat_char(buf, ch);
}

void ch_printf_color(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];      /* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_char_color(buf, ch);
}
void log_printf (char * fmt, ...)
{
        char buf [2*MAX_STRING_LENGTH];
        va_list args;
        va_start (args, fmt);
        vsprintf (buf, fmt, args);
        va_end (args);

        log_string (buf);
}

void do_llog( CHAR_DATA *ch, char *argument )
{
  int lines;
  char syscmd[100];
  FILE *fp;
  char buf[ MAX_STRING_LENGTH ];
  char letter;
  char *pt;
  int index;
  char filename[100];

  lines = atoi( argument );
  if( lines > 100 )
    lines = 100;
  if( lines < 20 )
    lines = 20;

  fclose( fpReserve );

  index = 1000;

  if( TEST_GAME )
    sprintf( filename, "../log_test/%d.log", index );
  else
    sprintf( filename, "../log/%d.log", index );

  fp = fopen( filename, "r");
  while( fp != NULL )
    {
    fclose( fp );
    index++;
    if( TEST_GAME )
      sprintf( filename, "../log_test/%d.log", index );
    else
      sprintf( filename, "../log/%d.log", index );
    fp = fopen( filename, "r");
    }

  index--;
  if( index < 1000 )
    {
    send_to_char( "No log file.\n\r", ch );
    fpReserve = fopen( NULL_FILE, "r");
    return;
    }

  sprintf( buf, "Log file used is #%d\n\r", index );
  send_to_char( buf, ch );

  if( TEST_GAME )
    sprintf( syscmd, "/usr/bin/tail -%d ../log_test/%d.log > llog.out", lines, index );
  else
    sprintf( syscmd, "/usr/bin/tail -%d ../log/%d.log > llog.out", lines, index );

  system( syscmd );

  fp = fopen( "llog.out", "r");
  if( fp == NULL )
    {
    send_to_char( "No llog generated.\n\r", ch );
    fpReserve = fopen( NULL_FILE, "r");
    return;
    }

  *buf='\0';
  pt = buf;
  letter = getc( fp );

  while( letter != EOF )
    {
    if( letter != '\r' )
      {
      *pt = letter;
      pt++;
      }
       /* Account for Unix's forgetting something in files */
    if( letter == '\n' )
      {
      *pt = '\r';
      pt++;
      }
    letter = getc( fp );
    }

  *pt = '\0';

  send_to_char( buf, ch );

  fclose( fp );

  fpReserve = fopen( NULL_FILE, "r");

  return;
}

void send_to_combat_char( char *txt, CHAR_DATA *ch )
{
  bool foundyou;
  char *buf;
  int leng;


  if( ch==NULL || IS_NPC( ch ) || ch->desc==NULL || ch->desc->character!=ch ||
      txt==NULL || *txt=='\0' )
    return;

  if( !ALLOW_OUTPUT )
    return;

  buf = ch->desc->combat_buffer;
  leng = ch->desc->combat_buffer_length;
  ch->desc->combat_buffer_index++;

  foundyou=scroll_you( ch->desc, txt, (ch->vt100_type/1000%10==1));

      if(foundyou)
        {
        if( ch->ansi!=0 )
          {
       leng=str_apd_max( buf, get_color_string( ch, COLOR_TEXT, VT102_BOLD ),
                    leng, COMBAT_STRING_LENGTH );
          }
        else
          leng = str_apd_max(buf,"\033[1m", leng, COMBAT_STRING_LENGTH );
        }
      else
        {
        if( ch->ansi!=0 )
          {
        leng =str_apd_max( buf, get_color_string( ch, COLOR_TEXT, VT102_DIM ),
                  leng,  COMBAT_STRING_LENGTH );
          }
        else
          leng = str_apd_max(buf,"\033[m", leng, COMBAT_STRING_LENGTH );
        }

  leng = str_apd_max( buf, txt, leng, COMBAT_STRING_LENGTH );
  ch->desc->combat_buffer_length = leng;

  if( ch->desc->combat_buffer_index > 8 )
    {
    char buft[ COMBAT_STRING_LENGTH+4 ];

    strcpy( buft, buf );
    *buf = '\0';
    ch->desc->combat_buffer_length = 0;
    ch->desc->combat_buffer_index = 0;

    send_to_char( buft, ch );
    }

  return;
}


bool scroll_you( DESCRIPTOR_DATA *d, const char *txi, bool youcheck)
{
  int cnt;
  CHAR_DATA *ch;
  bool foundyou;
  int youletter, lng;
  register char *pti;

  if( !youcheck )
    return( FALSE );

  foundyou=FALSE;
  youletter=0;

  if((d->connected!=CON_PLAYING && d->connected !=CON_EDITING)
     || d->original!=NULL)
    return(FALSE);
  ch=d->character;

  lng = strlen( txi );


    pti = (char *)txi;

  for(cnt=0;cnt<lng;cnt++, pti++)
    {
    if( youcheck)
      {
      if(foundyou==FALSE)
        {
        if(*pti=='Y' || *pti=='y')
          {
          foundyou=TRUE;
          youletter=1;
          }
        }
      else
        if( youletter==1)
        {
          if( *pti=='o' || *pti=='O')
            youletter=2;
          else
            foundyou=FALSE;
        }
        else
         if( youletter==2)
         {
          if( *pti=='u' || *pti=='U')
            {
            youletter=3;
            return( TRUE );
            }
          else
            foundyou=FALSE;
         }
      }
    }

  if( youletter == 3 )
    return( TRUE );
  else
    return( FALSE );
}


/* Days till character expires.  -1 is dead.  */
int character_expiration( CHAR_DATA *ch )
{
  struct tm ptm;
  struct timeval ctm;
  int pmonths, cmonths;

   if( ch->level < 1 )
      return( 1 );

   if (ch->level == 1 && ch->pcdata->last_time > 0)
    {
     gettimeofday( &ctm, NULL);
     return (((ch->pcdata->last_time + 24 * 60 * 60 * 7) - ctm.tv_sec)/(24*60*60));
    }
     
 
      if( ch->pcdata->last_time>0 )
        { 
        gettimeofday( &ctm, NULL);
	ptm = *localtime((const time_t *)&ch->pcdata->last_time);
        ptm.tm_min += ptm.tm_sec  / 60;
        ptm.tm_sec  = ptm.tm_sec  % 60;
        ptm.tm_hour+= ptm.tm_min  / 60;
        ptm.tm_min  = ptm.tm_min  % 60;
        ptm.tm_mday+= ptm.tm_hour / 24;
        ptm.tm_hour = ptm.tm_hour % 24;
        ptm.tm_mon += ptm.tm_mday / 30;
        ptm.tm_mday = ptm.tm_mday % 30;

        cmonths = ctm.tv_sec / 60 / 60 / 24 ;
        pmonths = ptm.tm_mday + 30 * ptm.tm_mon + 360*ptm.tm_year -25064 ;
        if( ch->pcdata->reincarnation > 0 )
          {
          cmonths = 36*30 - (cmonths-pmonths) ; /* Reincarnates get 3 years */
          if( cmonths < 1 )
            return( -1 );
          else
            return( cmonths );
          }
        else
          {
          if( ch->level==1 )
            cmonths = 7 - (cmonths-pmonths);
          else
            cmonths = ((ch->level*10)+30) - (cmonths-pmonths);
          if( cmonths < 1 )
            return( -1 );
          else
            return( cmonths );
          }
        }

   return( 100 );

}   


void scan_object_for_dup( CHAR_DATA *ch , OBJ_DATA *obj)
{
  OBJ_DATA *fobj;
  OBJ_REFERENCE *obj_ref;
  int ref_ind;
  int cv0, cv1, vnum;

  if( obj->index_reference[0] == 0 && obj->index_reference[1] == 0 )
    create_object_reference( obj );
  else
    {

    ref_ind = obj->index_reference[0] % 1000;
    cv0 = obj->index_reference[0];
    cv1 = obj->index_reference[1];
    vnum = obj->pIndexData->vnum;

    for( obj_ref=object_reference_hash[ref_ind]; obj_ref!=NULL; 
         obj_ref=obj_ref->next )
      {
      fobj = obj_ref->obj;
      if( fobj->index_reference[0] == cv0 )
       if( fobj->index_reference[1] == cv1 )
        if( fobj->pIndexData )
         if( fobj->pIndexData->vnum == vnum )
          if( fobj != obj )
              {
	      log_printf( "Dupe check would have invalidated %s (vnum %d) carried by %s\n", obj->name, vnum, ch->name);	
 	      /*
              SET_BIT( obj->extra_flags, ITEM_NOT_VALID );
              if( obj->wear_loc != WEAR_NONE )
	        remove_obj(ch,obj->wear_loc,TRUE, FALSE);
	      */
              break;
              }
      }
    }

  if( obj->first_content != NULL )
    scan_object_for_dup( ch, obj->first_content );

  if( obj->next_content != NULL )
    scan_object_for_dup( ch, obj->next_content );

  return;
}
      

void scan_for_duplication( CHAR_DATA *ch )
{

  if( ch->first_carrying != NULL )
    scan_object_for_dup( ch, ch->first_carrying );

  return;
}

void add_to_object_reference_hash( OBJ_DATA *obj )
{
  int ref_ind;
  OBJ_REFERENCE *new_ref;

  if( obj->index_reference[0] == 0 && obj->index_reference[1] == 0 )
    create_object_reference( obj );

  ref_ind = abs(obj->index_reference[0] % 1000);
  CREATE(new_ref, OBJ_REFERENCE, 1);
  new_ref->obj = obj;
  new_ref->next = object_reference_hash[ ref_ind ];
  object_reference_hash[ref_ind] = new_ref;

  return;
}



void remove_from_object_reference_hash( OBJ_DATA *obj)
{
  int ref_ind;
  OBJ_REFERENCE *prev_ref, *ref;

  if( obj->index_reference[0] == 0 && obj->index_reference[1] == 0 )
    return;

  ref_ind = abs(obj->index_reference[0]%1000);
  for( prev_ref=NULL, ref = object_reference_hash[ref_ind]; ref!=NULL;
       ref=ref->next )
    {
    if( ref->obj == obj )
      {
      if( prev_ref==NULL )
        object_reference_hash[ref_ind]=ref->next;
      else
        prev_ref->next = ref->next;
      DISPOSE( ref );
      return;
      }
    prev_ref = ref;
    }

  /* log_string( "Key index not found" ); */

  return;
}

void wipe_string( char *arg )
{
  char *pt;
  if( arg==NULL || *arg=='\0' )
    return;
  for( pt=arg; *pt!='\0'; pt++)
    *pt=' ';
  return;
}
