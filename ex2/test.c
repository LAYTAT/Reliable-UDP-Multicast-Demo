#include <stdio.h>
#include <stdlib.h>
#include "recv_dbg.h"

/* A simple test program for recv_dbg_init */

int main() {

    int i;
    int	decision;
    int counts[101];

    /* Test to see if the cutoff values are correct. Use machine index 1 */
    for ( i = 0; i <= 100; i++ ) {
    	recv_dbg_init( i, 1 );
    }

    printf("XXXXXXXXXXXXXXXXXXXXXXXXX\n");

    /* Test different machine indices */
    for ( i = 1; i <= 10; i++ ) {
    	recv_dbg_init( 1, i );
    }

    printf("XXXXXXXXXXXXXXXXXXXXXXXXX\n");

    /* The following test is designed to create a distribution of 
     * cutoff values */
    for ( i = 0; i <= 100; i++ ) counts[i] = 0;

    for ( i = 0; i < 100 * 10000; i++ ) {
        decision = rand() % 100 + 1 ;
        counts[decision]++;
    }

    for ( i = 0; i <= 100; i++ ) printf("%d %d\n", i, counts[i]);

    return 0;

} 
