  /*  Creates random maze and returns the vnum of farthest room from start */
int find_reverse( int, int, int );
int find_random( int, int, int );
int which_door( int, int );

CHAR_DATA *user;
int starting_room, size_x, size_y, size_z;
int last_door;

int mazegen( CHAR_DATA *ch, int start_room, int x_size, int y_size, int z_size, int seed)
  {
  int total_rooms, room_count, farthest_room, old_farthest;
  int x, y, z, door, dest, room;

  starting_room = start_room;

  if( x_size<1)
    x_size=1;
  if( y_size<1)
    y_size=1;
  if( z_size<1)
    z_size=1;
  total_rooms = x_size* y_size* z_size;
  size_x=x_size;
  size_y=y_size;
  size_z=z_size;
  old_farthest=0;
  farthest_room=starting_room;
  room_count=0;
  user=ch;

  for( room=starting_room; room< total_rooms + starting_room; room++)
    for( door=0; door<6; door++)
      set_exit( room, door, -1);
  room=starting_room;
  srand( seed );

  while( TRUE )
    {
    x=(room-starting_room)%size_x;
    y=((room-starting_room)/size_x)%size_y;
    z=(room-starting_room)/size_x/size_y;
    
    if( (dest=find_random( x, y, z)) >=0 )
      {
      set_exit( room, last_door, dest);
      room=dest;
      room_count++;
      }
    else
      if( (dest=find_reverse( x, y, z)) >=0 )
        {
        set_exit( room, last_door, dest);
        room=dest;
        room_count--;
        }
      else
        return( farthest_room);
    if( room_count > old_farthest )
      {
      farthest_room = room;
      old_farthest = room_count;
      }
      
    }
    return( farthest_room);

    
  }
  
int reverse( int in )
  {
  switch( in )
    {
    case 0: return (2);break;
    case 1: return (3);break;
    case 2: return (0);break;
    case 3: return (1);break;
    case 4: return (5);break;
    case 5: return (4);break;
    }
  return(-1);
  }

bool is_exit( int room, int door)
  {
	EXIT_DATA *pexit;
  if( room_index[room]==NULL)
    {
    log_string( "bad room.");
    return( FALSE);
    }
	pexit = room_index[room]->exit[door];
  if( pexit == NULL )
    return( FALSE );
	return( TRUE );
  }

void set_exit( int room, int door, int dest)
  {
    
	EXIT_DATA *pexit;
  if( room<1 || dest<-1 || room_index[ room ] == NULL || door<0 || door>5
           || ( dest != -1 && room_index[dest]==NULL))
    {
    char buf[200];
    sprintf( buf, "Bad room connect at %d door %d to %d", room, door, dest );
    log_string( buf );
    return;
    }

  pexit = room_index[ room ]->exit[door];
  if( pexit==NULL && dest>0)
    {
    CREATE(pexit, EXIT_DATA, 1);
    pexit->description	= STRALLOC("");
    pexit->keyword        = STRALLOC("");
    pexit->pvnum  = -1;
    pexit->key		= -1;
    pexit->vnum		= room;
    room_index[ room ]->exit[door]= pexit;
    top_exit++;
    }
  if( dest>0 )
    {
    pexit->exit_info=0;
    pexit->to_room = room_index[ dest ];
    }
  else
    {
    if( pexit==NULL)
      return;
    if (pexit->keyword != NULL )
      STRFREE (pexit->keyword );
    STRFREE (pexit->description );
    DISPOSE( pexit );
    room_index[ room ]->exit[door]=NULL;
    top_exit--;
    }
  return;
  }

int get_room( int x, int y, int z)
  {
  return( starting_room + x + ( y * size_x ) + ( z * size_x * size_y ) );
  }

int find_reverse( int x, int y, int z )
  {
  int door, dx, dy, dz;
  int room, dest;
  room= get_room( x, y, z);
  for( door=0; door<6; door++)
    if( !is_exit( room , door ))
      {
      dx=x;
      dy=y;
      dz=z;
      switch( door )
        {
        case 0: if( dy< size_y-1) dy++;break;
        case 1: if( dx< size_x-1) dx++;break;
        case 2: if( dy> 0) dy--;break;
        case 3: if( dx> 0) dx--;break;
        case 4: if( dz< size_z-1) dz++;break;
        case 5: if( dz> 0) dz--;break;
        }
      dest= get_room( dx, dy, dz);
      if( (x!=dx || y!=dy || z!=dz) && 
          is_exit( dest , reverse( door ) ) )
        {
        last_door=door;
        return( dest);
        }
      }
    return( -1 );
    }
        
      
int find_random( int x, int y, int z )
  {
  int door, dx, dy, dz;
  int start, direction , dest, room;
  bool found;

  found = FALSE;

  start=rand()%5;
  direction=(2*(rand()%2))-1;
  door=start;
  room = get_room( x, y, z);

  while( door!=start || !found )
    {
    found=TRUE;
    
    if( !is_exit( room, door ))
      {
      dx=x;
      dy=y;
      dz=z;
      switch( door )
        {
        case 0: if( dy< size_y-1) dy++;break;
        case 1: if( dx< size_x-1) dx++;break;
        case 2: if( dy> 0) dy--;break;
        case 3: if( dx> 0) dx--;break;
        case 4: if( dz< size_z-1) dz++;break;
        case 5: if( dz> 0) dz--;break;
        }
      if( (x!=dx || y!=dy || z!=dz) )
        {
        int tdoor;
        bool found;
        found=FALSE;
        dest= get_room( dx, dy, dz);
        for( tdoor=0; tdoor<6; tdoor++)
          if( is_exit( dest , tdoor ) )
            found=TRUE;
        if( !found)
          {
          last_door=door;
          return( dest);
          }
        }
      }
    door+=direction;
    if( door > 5 )
      door=0;
    if( door < 0 )
      door=5;
    }
  return( -1 );
  } 
      
int which_door( int room, int dest)
  {
	EXIT_DATA *pexit;
  int door;

  for( door=0; door<6; door++)
    {
	  pexit = room_index[ room ]->exit[door];
    if( pexit!=NULL)
      if( pexit->to_room->vnum == dest )
        return( door );
    }
  return( -1 );
  }

