//
// Created by 杜科 on 2020/10/15.
//
/**
 * 一些系统库
 */

#ifndef AOS_SYS_LIB_H
#define AOS_SYS_LIB_H

#ifndef AOS_TYPEs_H
#include <sys/types.h>
#endif


/* AOS 用户和系统双用库 */
_PROTOTYPE( int send_rec, (int src, Message_t *io_msg) );
_PROTOTYPE( int in_outbox, (Message_t *in_msg, Message_t *out_msg) );

/* AOS 系统库 */
_PROTOTYPE( int send, (int dest, Message_t* out_msg) );
_PROTOTYPE( int receive, (int src, Message_t* in_msg) );



#endif //AOS_SYS_LIB_H
