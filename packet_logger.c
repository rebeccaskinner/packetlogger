#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/netfilter.h>

typedef unsigned char byte;

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

#define EXPECT(x,y)  __builtin_expect((x),(y))
#define LIKELY(x)    EXPECT((x),TRUE)
#define UNLIKELY(x)  EXPECT((x),FALSE)

#define MAX_BUFFER_SIZE 65535

static byte nfq_buffer[MAX_BUFFER_SIZE] __attribute__((aligned));
static uint16_t queue_id = 0;

static struct nfq_handle* setup_netfilter_queue()
{
    struct nfq_handle* nfq_h = NULL;
    if(UNLIKELY(NULL == (nfq_h = nfq_open())))
    {
        fprintf(stderr,"Error: unable to initialize netfilter queue\n");
        goto error;
    }
    if(UNLIKELY(0 > nfq_unbind_pf(nfq_h,AF_INET)))
    {
        fprintf(stderr,"Error: unable to unbind netfilter handle\n");
        goto error;
    }
    if(UNLIKELY(0 > nfq_bind_pf(nfq_h,AF_INET)))
    {
        fprintf(stderr,"Error: unable to bind netfilter handle\n");
        goto error;
    }

    return nfq_h;

error:
    if(nfq_h) nfq_close(nfq_h);
    return NULL;
}

static struct nfq_q_handle* create_q_handle(struct nfq_handle* h, nfq_callback* cb, void* data)
{
    struct nfq_q_handle* qh = nfq_create_queue(h,queue_id++,cb,data);

    if(UNLIKELY(NULL == qh))
    {
        fprintf(stderr,"Error creating queue %d\n",queue_id-1);
        goto error;
    }
    if(UNLIKELY(-1 == nfq_set_mode(qh, NFQNL_COPY_PACKET, MAX_BUFFER_SIZE)))
    {
        fprintf(stderr,"Error setting NFQ copy mode\n");
        goto error;
    }

    return qh;

error:
    if(qh)
    {
        nfq_destroy_queue(qh);
    }
    return NULL;
}

static int log_packet(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* dat, void* data)
{
    struct nfqnl_msg_packet_hdr* nfq_hdr = nfq_get_msg_packet_hdr(dat);
    unsigned char* payload;
    int len = nfq_get_payload(dat,&payload);
    printf("---\n");
    printf("got a packet - len %d\n",len);
    nfq_set_verdict(qh, ntohl(nfq_hdr->packet_id), NF_ACCEPT, 0, NULL);
    return 0;
}

int main(int argc, char** argv)
{
    struct nfq_handle* nfq_h;
    struct nfq_q_handle* qh;
    int fd, rv;

    nfq_h = setup_netfilter_queue();
    if(!nfq_h) return -1;

    qh = create_q_handle(nfq_h, &log_packet, NULL);
    if(!qh) return -1;

    fd = nfq_fd(nfq_h);
    while((rv = recv(fd, nfq_buffer, sizeof(nfq_buffer), 0)))
    {
        nfq_handle_packet(nfq_h, (char*)nfq_buffer, rv);
    }

    return 0;
}
