//该文件源自http://www.cs.berkeley.edu/~istoica/tmp/i3-stable/i3_client/ping.c，进行了一些改进


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/icmp.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>
#include <linux/if.h>

#define RECV_BUF_LEN 128
#define PACKETSIZE  16

struct icmppacket
{
    struct icmphdr hdr;
	uint32_t addr;
	struct timeval time;
    char data[PACKETSIZE]; //不包括icmp协议头
};

/* IN_CKSUM: Internet checksum routine */
static uint16_t in_ping_cksum(uint16_t *addr, int len)
{
    int nleft = len;
    uint16_t *w = addr;
    uint16_t answer;
    int sum = 0;
    
    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we
     * add sequential 16 bit words to it, and at the end, fold back
     * all the carry bits from the top 16 bits into the lower 16 bits.
     */
    while( nleft > 1 )  
	{
		sum += *w++;
		nleft -= 2;
    }
    
    /* mop up an odd byte, if necessary */
    if (nleft == 1) 
	{
		uint16_t u = 0;
		*(uint8_t *)(&u) = *(uint8_t *)w ;
		sum += u;
    }
    
    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);			/* add carry */
    answer = ~sum;			/* truncate to 16 bits */
    return (answer);
}

/* Fill in echo request packet and send to destination */
static int send_echo_request(int socket, struct sockaddr_in* to_addr, int seq) 
{
    struct icmppacket echoReq;
    static int nId = 1;
    int	i = 0;
	int	nRet = 0;
	
    /* fill in echo request packet */
    /* header */
    echoReq.hdr.type		= ICMP_ECHO;
    echoReq.hdr.code 		= 0;
    echoReq.hdr.checksum	= 0;
    echoReq.hdr.un.echo.id 	= htons(getpid() & 0xffff);;
    echoReq.hdr.un.echo.sequence = htons(seq);
	
	/* target of ICMP ping */
    echoReq.addr = to_addr->sin_addr.s_addr;
	gettimeofday(&echoReq.time, 0);

    /* fill random data */
    for (i = 0; i < PACKETSIZE; i++)
    {
		echoReq.data[i] = (char)(random()&0x000000ff);
    }
	
    /* compute checksum */
    echoReq.hdr.checksum = in_ping_cksum((uint16_t *)&echoReq, sizeof(struct icmppacket));
    
    /* Send the echo request */
    nRet = sendto(socket, (void*)&echoReq, sizeof(struct icmppacket), 0,
	    (struct sockaddr *)to_addr, sizeof(struct sockaddr_in));
    
    if (nRet < 0)
    {
		printf("Not enough bytes send, ignoring ICMP packet seq: %d\n", seq);
    }
	
    return nRet;
}

/* Process received ICMP packet: code is self-documenting */
static int recv_echo_reply(int socket)
{	
    struct iphdr *ip_header;
    struct icmppacket *echoReply;
	
    struct sockaddr_in addr;
    char recv_data[RECV_BUF_LEN];
    int nAddrLen = sizeof(struct sockaddr_in);
    int nRet;
    
    nRet = recvfrom(socket, (char *)recv_data, RECV_BUF_LEN,
	    	    0, (struct sockaddr*)&addr, &nAddrLen);
    ip_header = (struct iphdr *) recv_data;
    echoReply = (struct icmppacket *) (recv_data + sizeof(struct iphdr));
    
    if (nRet < 0) 
	{
		printf("echo_reply errno=%d", errno);
    }
    else if (nRet < sizeof(struct iphdr) + sizeof(struct icmphdr)) 
	{
		printf("Not enough bytes received\n");
    }
    else if ((ip_header->protocol != IPPROTO_ICMP)) 
	{
		printf("Incorrect protocol type received\n");
    }
    else if (echoReply->hdr.type != ICMP_ECHOREPLY) 
	{
		printf("Not ECHO_REPLY message\n");
    }
    else if (addr.sin_addr.s_addr != echoReply->addr) 
	{
		struct sockaddr_in in_addr;
		in_addr.sin_addr.s_addr = echoReply->addr;
		printf("send address %s does not match recv address %s\n", inet_ntoa(addr.sin_addr), inet_ntoa(in_addr.sin_addr));
    }
    else //normal reply
	{
		uint32_t ret_addr = echoReply->addr;
		uint16_t ret_seq  = ntohs(echoReply->hdr.un.echo.sequence);
		uint8_t  ret_ttl  = ip_header->ttl;
		uint16_t ret_len  = nRet - (sizeof(struct iphdr) + sizeof(struct icmphdr));

		struct timeval current_time;
		gettimeofday(&current_time, 0);
		
		struct timeval elapse_time;
		elapse_time.tv_sec = current_time.tv_sec - echoReply->time.tv_sec; 
		elapse_time.tv_usec = current_time.tv_usec - echoReply->time.tv_usec; //ms
					
		printf("%d bytes from %s: icmp_req=%d ttl=%d time=%d.%-2.0f ms!!!\n",
			ret_len, inet_ntoa(addr.sin_addr), ret_seq, ret_ttl, (int)elapse_time.tv_sec*1000, (float)elapse_time.tv_usec/1000);
		return 0;
    }

    return -1;
}

/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*    return 0 is ping Ok, return 1 is ping not OK.                ---*/
/*--------------------------------------------------------------------*/
int Ping(const char* adress, const uint16_t port, const uint8_t retry)
{
	int ping_socket;
    struct sockaddr_in addr_ping, addr_host;
	int ret;
	int nRet;

    ping_socket = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if ( ping_socket < 0 )
    {
        printf("socket() error=%d\n", errno);
        return -1;
    }
    addr_host.sin_family = AF_INET;
    addr_host.sin_port = htons(0);
	addr_host.sin_addr.s_addr = htonl(INADDR_ANY);
	
    ret = bind(ping_socket, (const struct sockaddr *)&addr_host, sizeof(struct sockaddr_in));
    if (ret != 0) 
	{
		printf("bind() error=%d\n", errno);
    }
	
	//设置ttl为255
    const int val=255;
    if ( setsockopt(ping_socket, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        printf("Set TTL option, error=%d\n", errno);
    }
	
	struct timeval tv;
	tv.tv_sec = 0; 
	tv.tv_usec = 100*1000; //100ms，因为是内网，接收时延100ms足够了
	setsockopt(ping_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	
	tv.tv_sec = 0; 
	tv.tv_usec = 10*1000; //10ms，不到20个字节发送时延10ms足够了
	setsockopt(ping_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    memset(&addr_ping, 0, sizeof(struct sockaddr_in));
    addr_ping.sin_family= AF_INET;
    addr_ping.sin_port = htons(port);
	if(inet_aton(adress, &addr_ping.sin_addr) == 0)
	{
        printf("inet_aton error=%d\n", errno);
        return -1;
	}

	//ping retry times
    int loop;
    for (loop=0; loop < retry; loop++)
    {
		nRet = send_echo_request(ping_socket, &addr_ping, loop+1);
		if(nRet < sizeof(struct iphdr) + sizeof(struct icmphdr))
		{
			continue;
		}

		if(-1 == recv_echo_reply(ping_socket))
		{
			continue;
		}
		else
		{
			return 1; //成功1次就返回，多了影响效率
		}
    }

    return 0;
}

int GetWiredIPAddress(char *ipaddr, uint32_t ipaddr_len)
{
	int i=0;
  	int sockfd;
  	struct ifconf ifconf;
  	char buf[512];
  	struct ifreq *ifreq;

  	//初始化ifconf
  	ifconf.ifc_len = 512;
  	ifconf.ifc_buf = buf;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
		printf("socket failed, errno is %d\n", errno);
        return -1;
    }
  	ioctl(sockfd, SIOCGIFCONF, &ifconf);    //获取所有接口信息

 	 //接下来一个一个的获取IP地址
  	ifreq = (struct ifreq*)buf;
  	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
  	{
	  	if(ifreq->ifr_flags == AF_INET)
		{//for ipv4
		      if(0 == strncasecmp(ifreq->ifr_name, "ppp0", strlen("ppp0"))) //pppoe拨号虚拟网卡
		      {
					printf("name = [%s]\n", ifreq->ifr_name);
					strncpy(ipaddr,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr),ipaddr_len);
					ipaddr[ipaddr_len-1] = '\0';
					break;
		      }
		      if(0 == strncasecmp(ifreq->ifr_name, "eth0", strlen("eth0"))) //本地网卡
		      {
					printf("name = [%s]\n", ifreq->ifr_name);
					strncpy(ipaddr,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr),ipaddr_len);
					ipaddr[ipaddr_len-1] = '\0';
					break;
		      }
		      ifreq++;
		}
  	}

    printf("get ip addr: %s, the len is %d\n",ipaddr, ipaddr_len);
    return 0;
}

//随机端口，一个浏览器生命周期中仅用一个端口
//如果该端口已经占用，服务器会找到一个能连接的客户端的端口，一般向上偏移
uint32_t GetRand(uint32_t seed_me, uint32_t range) 
{
    srandom(time(NULL)+seed_me);
	return 1 + range * (random() / (RAND_MAX + 1.0));
}

