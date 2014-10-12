#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "zmq.h"
#include "zmqserver.h"
#include <netdb.h>
// #include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <assert.h>

#define SERVER "10.0.0.17"

/* This is ZeroMQ daemon for communication channel */
int main (int argc, char const *argv[]) {


    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    while (1) {
        char buffer [10];
        zmq_recv (responder, buffer, 10, 0);
        printf ("Received Hello\n");
        sleep (1);          //  Do some 'work'
        zmq_send (responder, "World", 5, 0);
    }
    return 0;

}

// void getIP(char *ip) {
// 	struct ifaddrs *ifaddr, *ifa;
//     int family, s;
//     char host[NI_MAXHOST];

//     if (getifaddrs(&ifaddr) == -1) 
//         perror("getifaddrs");

//     for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
//     {
//         if (ifa->ifa_addr == NULL)
//             continue;  

//         s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),
//         				host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

//         if((strcmp(ifa->ifa_name,"wlan0")==0)&&(ifa->ifa_addr->sa_family==AF_INET)){
//             if (s != 0)
//                 printf("getnameinfo() failed: %s\n", gai_strerror(s));
//             // printf("\t  Address : <%s>\n", host); 
//             freeifaddrs(ifaddr);
//             strcpy(ip, host);
//             return;
//         }
//     }

//     freeifaddrs(ifaddr);
//     strcpy(ip, "10.0.0.8");
//     return ; // just for demo purpose
// }
