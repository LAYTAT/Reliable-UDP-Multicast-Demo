#ifndef CS417_DBG
#define CS417_DBG

#include <sys/types.h>
#include <sys/socket.h> 

/* Returns 0 if the packet was dropped (and should be treated as omitted by
 * your program), and the number of bytes received otherwise. */
int 	recv_dbg(int s, char *buf, int len, int flags);

void	recv_dbg_init(int percent, int machine_index);

#endif  /* CS417_DBG */


