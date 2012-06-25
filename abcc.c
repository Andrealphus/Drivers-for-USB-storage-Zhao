#include<linux/unistd.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("mq110");

static void __exit print_string_exit(void);
static int __init print_string_init(void);
static void print_string(char *str)
{
    struct tty_struct *my_tty; int a;
    my_tty = current->signal->tty;
    if (my_tty != NULL)
    {
      a= my_tty->driver->ops->write(my_tty,str,strlen(str));
    //   a= my_tty->driver->ops->write(my_tty,"\n",1);
    }
}
static int __init print_string_init(void)
{
    print_string("Hello world!");
__NR_delete_module("abcc");
print_string_exit();
    return 0;
}
static void __exit print_string_exit(void)
{
    print_string("Goodbye world!");
}
module_init(print_string_init);
module_exit(print_string_exit); 
