#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

// code for a client connecting to a server
// namely a stream socket to www.example.com on port 80 (http)
// either IPv4 or IPv6

int get_ipstring(char *iface, char *ipstr, int strlen);

int get_ipstring(char *iface, char *ipstr, int strlen)
{
int sockfd;  
struct ifreq ifr;
sockfd = socket(AF_INET, SOCK_DGRAM, 0);
ifr.ifr_addr.sa_family = AF_INET;
strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
ioctl(sockfd, SIOCGIFADDR, &ifr);
close(sockfd);
strncpy(ipstr, (const char *)inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), strlen);

return 0;
}

#ifdef TESTDRIVER
int main(int argc, char **argv)
{
char str[20];
	if (argc < 2)
	{
		fprintf(stderr, "syntax: %s <lan i/f name>\n", argv[0]);
		exit(0);
	}
	get_ipstring(argv[1], str, sizeof(str));
	printf("%s\n", str);

	return 0;
}
#endif
