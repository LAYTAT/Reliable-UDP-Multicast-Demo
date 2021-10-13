#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTES_TO_ZERO 16

static int cutoff = 20; /* default is 20% loss */
static struct timeval init_time;
static struct timeval current_time;

void recv_dbg_init(int percent, int machine_index )
{
        int    seed;

        /* percent is in integer form (i.e. 1 = 1%, 5 = 5%) */
        cutoff  = percent;
        if( cutoff < 0 ) cutoff = 0;
        else if( cutoff > 100 ) cutoff = 100;

        gettimeofday( &init_time, NULL );
        seed = init_time.tv_sec + init_time.tv_usec + machine_index;
        srand( seed );

        printf("\nrecv_dbg_init: percent loss = %d, seed = %d \n", cutoff, seed);
}

int recv_dbg(int s, char *buf, int len, int flags)
{
        int ret;
        int decision;
		int i;

        decision = rand() % 100 + 1;
	
        gettimeofday( &current_time, NULL );
        if( current_time.tv_sec - init_time.tv_sec > 180 ) 
        {
            printf("recv_dbg: time is up - killing the process \n");
            exit( 1 );
        }

        ret = recv( s, buf, len, flags );
        if( ( ret > 0 ) && ( cutoff > 0 ) && ( decision <= cutoff ) )
        {
                for( i = 0; i < ret && i < BYTES_TO_ZERO ; i++) buf[i] = 0;
                return(0);
        }
        return( ret );
}

