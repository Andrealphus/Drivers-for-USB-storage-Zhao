#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>

#include <unistd.h> 



void a()
{
int aaa= 0;
int hotplug= 0;
const int buffersize = 1024 ;      //1K 
/*

*/
struct sockaddr_nl socket_usb;
char result[1000];
int key; 
//FILE *stream;
bzero(&result, 1000);
memset(&socket_usb, 0x00, sizeof(struct sockaddr_nl));
socket_usb.nl_family = AF_NETLINK;
socket_usb.nl_pid = getpid();
socket_usb.nl_groups = 1;//1;
//创建套接字
hotplug = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);

setsockopt(hotplug, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
//绑定(struct sockaddr *)
aaa = bind(hotplug,  (struct sockaddr *)&socket_usb, sizeof(struct sockaddr_nl));
printf("ok1\n");
while(1)
{
bzero(&result, 1000);
recv(hotplug, &result, sizeof(result), 0);
printf("ok\n");

printf("%s\n",result);
if(!memcmp(result,"add@",4) && !memcmp(&result[strlen(result) - 4],"/sdb1",4))
{
printf(".......\n");

sleep(5);

system("cp -r  /media/. /home/bcxc/upan2");
printf("final cp\n");
}
printf("here\n");
}
sleep(10);
/**/
printf("cp\n");
}
int main(int argc,char *argv[])
{

a();
return 1;
}
 
