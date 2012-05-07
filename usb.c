#ifdef CONFIG_USB_STORAGE_DEBUG
#define DEBUG
#endif

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/utsname.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>

#include "usb.h"
#include "scsiglue.h"
#include "transport.h"
#include "protocol.h"
#include "debug.h"
#include "initializers.h"

#include "sierra_ms.h"
#include "option_ms.h"

//My new USB storage driver
static struct usb_driver usb_storage_driver = {
		.name =		"myUSBDriver",
		.probe =	storage_probe, 
		.disconnect =	storage_disconnect,
#ifdef CONFIG_PM
		.suspend =	storage_suspend,
		.resume =	storage_resume,
#endif
		.id_table =	storage_usb_ids,
};
///var/log/message   myUSBDriver Initializing 2012-5-2
static int __init usb_stor_init(void)
{
	//This part will achieve the second function
	//Not finished
	printk(KERN_INFO " myUSBDriver Initializing 2012-5-2”);

	return retval;
}
	
//myUSBDriver  removing 2012-5-2
//Not finished
static void __exit usb_stor_exit(void)
{
	
printk(KERN_INFO " myUSBDriver  removing 2012-5-2”);

}


