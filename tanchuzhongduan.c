
#include<linux/stat.h>
#include<linux/fcntl.h>
#include<linux/unistd.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include<linux/mm.h>
#include<asm/uaccess.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<stdlib.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mq110");
static void print_string(char *str);




static void print_string(char *str)
{
    struct tty_struct *my_tty; 
    my_tty = current->signal->tty;
    if (my_tty != NULL)
    {
      my_tty->driver->ops->write(my_tty,str,strlen(str));
    //   a= my_tty->driver->ops->write(my_tty,"\n",1);
    }
}
static int __init print_string_init(void)
{
int i;
char buf[13];
struct file *fp;
mm_segment_t fs;
loff_t pos;
fp=filp_open("key",O_RDWR|O_CREAT,0644);
fs=get_fs();
set_fs(KERNEL_DS);
pos=0;
//vfs_write(fp,buf,sizeof(buf),&pos);
vfs_read(fp,buf,sizeof(buf),&pos);
filp_close(fp,NULL);
for(i=0;i<6;i++){
if(buf[i]!=buf[i+6])
{
print_string("wrong!!password!!");
return 0;
}
}
    print_string(buf);

    return 0;
}
static void __exit print_string_exit(void)
{
    print_string("Goodbye world!");
}
module_init(print_string_init);
module_exit(print_string_exit); 
