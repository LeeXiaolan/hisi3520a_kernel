/*
 *  COPYRIGHT NOTICE
 *
 *  Copyright 2011 Emutex Ltd. All rights reserved.
 *
 *  tnk info structure
 */

#ifndef __TNKTCP_H
#define __TNKTCP_H

#ifndef UNIT_TEST
#include <net/tnkdrv.h>
#endif

#include "tnkinfo.h"

void tnk_tcp_init(void);
void tnk_tcp_shutdown(void);
int tnk_tcp_prepare(struct sock *sk, struct sk_buff *skb, int inbound);
int tnk_tcp_open(struct sock *sk);
int tnk_tcp_close(struct sock *sk, int graceful);
int tnk_tcp_receive(struct sock *sk, uint32_t *seq, long timeo,
		    int target, struct iovec *to, size_t len);
int tnk_tcp_read_sock(struct sock *sk, read_descriptor_t *desc,
		      tnk_read_actor_t recv_actor);

int tnk_tcp_sendpages(struct sock *sk, struct page **pages, int poffset,
		      size_t psize, int flags);
int tnk_tcp_sendmsg(struct sock *sk, struct iovec *from, int len, int flags);
int tnk_tcp_update(struct sock *sk, int len);
int tnk_tcp_wait_data(struct sock *sk, long *timeo);

#endif
