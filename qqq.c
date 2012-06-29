#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>

#include <unistd.h> 

#define UEVENT_BUFFER_MAX       (1024*15)   //15K


void usbmonitor()
{
int aiResult= 0;
int aiHotplug_sock= 0;
const int buffersize = 1024 ;      //1K 
/*

*/
struct sockaddr_nl astNLaddr;
char apcRecBuf[UEVENT_BUFFER_MAX];
int key; 
FILE *stream;
bzero(&apcRecBuf, UEVENT_BUFFER_MAX);
memset(&astNLaddr, 0x00, sizeof(struct sockaddr_nl));
astNLaddr.nl_family = AF_NETLINK;
astNLaddr.nl_pid = getpid();
astNLaddr.nl_groups = 1;
aiHotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
setsockopt(aiHotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
aiResult = bind(aiHotplug_sock, (struct sockaddr *) &astNLaddr, sizeof(struct sockaddr_nl));
while(1)
{
bzero(&apcRecBuf, UEVENT_BUFFER_MAX);
recv(aiHotplug_sock, &apcRecBuf, sizeof(apcRecBuf), 0);

if(!memcmp(apcRecBuf,"add@",4) && !memcmp(&apcRecBuf[strlen(apcRecBuf) - 4],"/sdb",4))
{

system("rmmod usb-storage");
printf("please input key contion 6 num");
scanf("%d",a);
//if
break;
}
}
sleep(1);
/**/
system("cp -r  /media/. /home/bcxc/upan");
}
int main(int argc,char *argv[])
{

usbmonitor();
return 1;
}
 
