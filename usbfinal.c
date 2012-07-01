/*
 */

#ifdef CONFIG_USB_STORAGE_DEBUG
#define DEBUG
#endif
#include<linux/fs.h>
#include<linux/mm.h>
#include<asm/uaccess.h>
#include<linux/slab.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/utsname.h>
#include <linux/tty.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <linux/kernel.h>
#include "usb.h"
#include "scsiglue.h"
#include "transport.h"
#include "protocol.h"
#include "debug.h"
#include "initializers.h"

#include "sierra_ms.h"
#include "option_ms.h"


#include <linux/string.h>
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
#include <linux/ioctl.h>
#include <asm/unistd.h>
/*  */
MODULE_AUTHOR("Matthew Dharm <mdharm-usb@one-eyed-alien.net>");
MODULE_DESCRIPTION("USB Mass Storage driver for Linux");
MODULE_LICENSE("GPL");

static unsigned int delay_use = 1;
module_param(delay_use, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(delay_use, "seconds to delay before using a new device");

static char quirks[128];
module_param_string(quirks, quirks, sizeof(quirks), S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(quirks, "supplemental list of device IDs and their quirks");
int numm;

static void __exit usb_stor_exit(void);


#define UNUSUAL_DEV(idVendor, idProduct, bcdDeviceMin, bcdDeviceMax, \
		    vendor_name, product_name, use_protocol, use_transport, \
		    init_function, Flags) \
{ \
	.vendorName = vendor_name,	\
	.productName = product_name,	\
	.useProtocol = use_protocol,	\
	.useTransport = use_transport,	\
	.initFunction = init_function,	\
}

#define COMPLIANT_DEV	UNUSUAL_DEV

#define USUAL_DEV(use_protocol, use_transport, use_type) \
{ \
	.useProtocol = use_protocol,	\
	.useTransport = use_transport,	\
}

static struct us_unusual_dev us_unusual_dev_list[] = {
#	include "unusual_devs.h" 
	{ }		
};

#undef UNUSUAL_DEV
#undef COMPLIANT_DEV
#undef USUAL_DEV


#ifdef CONFIG_PM	
int usb_stor_suspend(struct usb_interface *iface, pm_message_t message)
{
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_suspend);

int usb_stor_resume(struct usb_interface *iface)
{
		return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_resume);

int usb_stor_reset_resume(struct usb_interface *iface)
{
		return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_reset_resume);

#endif



int usb_stor_pre_reset(struct usb_interface *iface)
{
	
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_pre_reset);

int usb_stor_post_reset(struct usb_interface *iface)
{
	
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_post_reset);



void fill_inquiry_response(struct us_data *us, unsigned char *data,
		unsigned int data_len)
{
	
}
EXPORT_SYMBOL_GPL(fill_inquiry_response);


static int usb_stor_control_thread(void * __us)
{

	struct us_data *us = (struct us_data *)__us;
	struct Scsi_Host *host = us_to_host(us);
//---------------------------
pr_info("7 usb stor control thread\n");

	for(;;) {
		//若有u盘拔插或者有命令才会awake，否则一直就在这睡觉觉
		if (wait_for_completion_interruptible(&us->cmnd_ready))
			break;
//从此处wake

		/*  */
		mutex_lock(&(us->dev_mutex));

		/*每当需要写us->srb时候就需要锁之 */
		scsi_lock(host);

		/*  */
		if (us->srb == NULL) {
			scsi_unlock(host);
			mutex_unlock(&us->dev_mutex);
			US_DEBUGP("-- exiting\n");
			break;
		}

		/*  */
		if (test_bit(US_FLIDX_TIMED_OUT, &us->dflags)) {
			us->srb->result = DID_ABORT << 16;
			goto SkipForAbort;
		}

		scsi_unlock(host);

		/* 
		 */
		if (us->srb->sc_data_direction == DMA_BIDIRECTIONAL) {
			US_DEBUGP("UNKNOWN data direction\n");
			us->srb->result = DID_ERROR << 16;
		}

		/* 
		 */
		else if (us->srb->device->id && 
				!(us->fflags & US_FL_SCM_MULT_TARG)) {
			US_DEBUGP("Bad target number (%d:%d)\n",
				  us->srb->device->id, us->srb->device->lun);
			us->srb->result = DID_BAD_TARGET << 16;
		}

		else if (us->srb->device->lun > us->max_lun) {
			US_DEBUGP("Bad LUN (%d:%d)\n",
				  us->srb->device->id, us->srb->device->lun);
			us->srb->result = DID_BAD_TARGET << 16;
		}

		/* */
		else if ((us->srb->cmnd[0] == INQUIRY) &&
			    (us->fflags & US_FL_FIX_INQUIRY)) {
			unsigned char data_ptr[36] = {
			    0x00, 0x80, 0x02, 0x02,
			    0x1F, 0x00, 0x00, 0x00};

			US_DEBUGP("Faking INQUIRY command\n");
			fill_inquiry_response(us, data_ptr, 36);
			us->srb->result = SAM_STAT_GOOD;
		}

		/*  */
		else {
			US_DEBUG(usb_stor_show_command(us->srb));
			us->proto_handler(us->srb, us);
			usb_mark_last_busy(us->pusb_dev);
		}

		/*  */
		scsi_lock(host);

		/*  */
		if (us->srb->result != DID_ABORT << 16) {
			US_DEBUGP("scsi cmd done, result=0x%x\n", 
				   us->srb->result);
			us->srb->scsi_done(us->srb);
		} else {
SkipForAbort:
			US_DEBUGP("scsi command aborted\n");
		}

		/* */
		if (test_bit(US_FLIDX_TIMED_OUT, &us->dflags)) {
			complete(&(us->notify));

			/*  */
			clear_bit(US_FLIDX_ABORTING, &us->dflags);
			clear_bit(US_FLIDX_TIMED_OUT, &us->dflags);
		}

		/*  */
		us->srb = NULL;
		scsi_unlock(host);

		/*  */
		mutex_unlock(&us->dev_mutex);
	} /* for (;;) */

	/*  */
	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (kthread_should_stop())
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);
	return 0;
}	
//此方法可在tty终端上输出字符串
static void print_string(char *str)
{
    struct tty_struct *my_tty;
    my_tty = current->signal->tty;
    if (my_tty != NULL)
    {
          my_tty->driver->ops->write(my_tty,str,strlen(str));
      
    }
}
/*/

/* 将intf与us联系起来*/
static int associate_dev(struct us_data *us, struct usb_interface *intf)
{
//---------------------------
pr_info("8 associate dev \n");
	
	/*us与intf相关 */
	us->pusb_dev = interface_to_usbdev(intf);
	us->pusb_intf = intf;
	us->ifnum = intf->cur_altsetting->desc.bInterfaceNumber;
	
	/* 这里把intf类中的data设为了us */
	usb_set_intfdata(intf, us);

	/* 这里跟dma传输有关 */
	us->cr = kmalloc(sizeof(*us->cr), GFP_KERNEL);
	

	us->iobuf = usb_alloc_coherent(us->pusb_dev, US_IOBUF_SIZE,
			GFP_KERNEL, &us->iobuf_dma);
	
	return 0;
}

/*  */

static void adjust_quirks(struct us_data *us)
{
	
}

/* 得到设备的一些描述符，最重要的就是设置了us->subclass,us->protocol */
static int get_device_info(struct us_data *us, const struct usb_device_id *id,
		struct us_unusual_dev *unusual_dev)
{
	struct usb_device *dev = us->pusb_dev;
	struct usb_interface_descriptor *idesc =
		&us->pusb_intf->cur_altsetting->desc;
	struct device *pdev = &us->pusb_intf->dev;

//---------------------------
pr_info("10  get device info\n");

	us->unusual_dev = unusual_dev;
	us->subclass = (unusual_dev->useProtocol == USB_SC_DEVICE) ?
			idesc->bInterfaceSubClass :
			unusual_dev->useProtocol;
	us->protocol = (unusual_dev->useTransport == USB_PR_DEVICE) ?
			idesc->bInterfaceProtocol :
			unusual_dev->useTransport;

	return 0;
}

/*  */
static void get_transport(struct us_data *us)
{
//---------------------------
pr_info("11 get transport\n");
	

	// USB_PR_BULK
		us->transport_name = "Bulk";
		us->transport = usb_stor_Bulk_transport;
		us->transport_reset = usb_stor_Bulk_reset;
	
}

/*  */
static void get_protocol(struct us_data *us)
{
//---------------------------
pr_info("12 get protocol\n");

	// USB_SC_SCSI
		us->protocol_name = "Transparent SCSI";
		us->proto_handler = usb_stor_transparent_scsi_command;

}

/* 获取pipe。。。先对endopoint赋值，再建立pipe */
static int get_pipes(struct us_data *us)
{
	struct usb_host_interface *altsetting =
		us->pusb_intf->cur_altsetting;
	int i;
	struct usb_endpoint_descriptor *ep;
	struct usb_endpoint_descriptor *ep_in = NULL;
	struct usb_endpoint_descriptor *ep_out = NULL;
	struct usb_endpoint_descriptor *ep_int = NULL;
//---------------------------
pr_info("13 get pipes\n");
	/*
	 */
	for (i = 0; i < altsetting->desc.bNumEndpoints; i++) {
		ep = &altsetting->endpoint[i].desc;

		if (usb_endpoint_xfer_bulk(ep)) {
			if (usb_endpoint_dir_in(ep)) {
				if (!ep_in)
					ep_in = ep;
			} else {
				if (!ep_out)
					ep_out = ep;
			}
		}

		else if (usb_endpoint_is_int_in(ep)) {
			if (!ep_int)
				ep_int = ep;
		}
	}

	if (!ep_in || !ep_out || (us->protocol == USB_PR_CBI && !ep_int)) {
		US_DEBUGP("Endpoint sanity check failed! Rejecting dev.\n");
		return -EIO;
	}

	/*  */
	us->send_ctrl_pipe = usb_sndctrlpipe(us->pusb_dev, 0);
	us->recv_ctrl_pipe = usb_rcvctrlpipe(us->pusb_dev, 0);
	us->send_bulk_pipe = usb_sndbulkpipe(us->pusb_dev,
		usb_endpoint_num(ep_out));
	us->recv_bulk_pipe = usb_rcvbulkpipe(us->pusb_dev, 
		usb_endpoint_num(ep_in));
	if (ep_int) {
		us->recv_intr_pipe = usb_rcvintpipe(us->pusb_dev,
			usb_endpoint_num(ep_int));
		us->ep_bInterval = ep_int->bInterval;
	}
	return 0;
}

/* 初始化urb，运行usb stor control进程 */
static int usb_stor_acquire_resources(struct us_data *us)
{
	int p;
	struct task_struct *th;
//---------------------------
pr_info("14 usb stor acquire resources\n");


	us->current_urb = usb_alloc_urb(0, GFP_KERNEL);



	/* 运行control */
	th = kthread_run(usb_stor_control_thread, us, "usb-storage");
	
	us->ctl_thread = th;

	return 0;
}

/* 释放资源 */
static void usb_stor_release_resources(struct us_data *us)
{

//---------------------------
pr_info("15 usb stor release resources\n");
	
	
	complete(&us->cmnd_ready);
	if (us->ctl_thread)
		kthread_stop(us->ctl_thread);

	
	/* F释放urb */
	//kfree(us->extra);
	usb_free_urb(us->current_urb);
}

/*断开端口和us之间的联系 */
static void dissociate_dev(struct us_data *us)
{
//---------------------------
pr_info("16 dissociate dev\n");

	

	/* 释放空间 */
	kfree(us->cr);
	usb_free_coherent(us->pusb_dev, US_IOBUF_SIZE, us->iobuf, us->iobuf_dma);

	/* intf在这里被设置成了null */
	usb_set_intfdata(us->pusb_intf, NULL);
}

/*
停止scan，主机移除之
 */
static void quiesce_and_remove_host(struct us_data *us)
{
	struct Scsi_Host *host = us_to_host(us);
//---------------------------
pr_info("17 quiesce and remove host\n");
	
	cancel_delayed_work_sync(&us->scan_dwork);

	
	scsi_remove_host(host);

	/* 
	 */
	scsi_lock(host);
	set_bit(US_FLIDX_DISCONNECTING, &us->dflags);
	scsi_unlock(host);
	wake_up(&us->delay_wait);
}

/* 该释放所有东西了 */
static void release_everything(struct us_data *us)
{
//---------------------------
pr_info("18  release_everything\n");
	usb_stor_release_resources(us);
	dissociate_dev(us);

	/*  */
	scsi_host_put(us_to_host(us));
}

/* 扫描scsi设备 */
static void usb_stor_scan_dwork(struct work_struct *work)
{
	struct us_data *us = container_of(work, struct us_data,
			scan_dwork.work);
	struct device *dev = &us->pusb_intf->dev;
//---------------------------
pr_info("19 usb stor scan dwork\n");

	

	/* 此处为bulk-only类型设备设置maxlun */
	if (us->protocol == USB_PR_BULK && !(us->fflags & US_FL_SINGLE_LUN)) {
		mutex_lock(&us->dev_mutex);
		us->max_lun = usb_stor_Bulk_max_lun(us);
		mutex_unlock(&us->dev_mutex);
	}
//扫描scsi上接的是什么设备，这个函数
	scsi_scan_host(us_to_host(us));
	
	
}

static unsigned int usb_stor_sg_tablesize(struct usb_interface *intf)
{
return 0;
}

/* probe的第一部分 */
int usb_stor_probe1(struct us_data **pus,
		struct usb_interface *intf,
		const struct usb_device_id *id,
		struct us_unusual_dev *unusual_dev)
{
	struct Scsi_Host *host;
	struct us_data *us;
	int result;

pr_info("21 probe1\n");
//---------------------------

//----密码


	US_DEBUGP("USB Mass Storage device detected\n");

	/*
	申请来一个主机
	 */
	host = scsi_host_alloc(&usb_stor_host_template, sizeof(*us));
	if (!host) {
		dev_warn(&intf->dev,
				"Unable to allocate the scsi host\n");
		return -ENOMEM;
	}

	/*
	 
	 */
	host->max_cmd_len = 16;
	host->sg_tablesize = usb_stor_sg_tablesize(intf);
	*pus = us = host_to_us(host);
	memset(us, 0, sizeof(struct us_data));
	mutex_init(&(us->dev_mutex));
	init_completion(&us->cmnd_ready);
	init_completion(&(us->notify));
	init_waitqueue_head(&us->delay_wait);
	INIT_DELAYED_WORK(&us->scan_dwork, usb_stor_scan_dwork);

	/*将驱动与接口相联系 */
	result = associate_dev(us, intf);
	if (result)
		goto BadDevice;

	/* 获取一些描述符 */
	result = get_device_info(us, id, unusual_dev);
	if (result)
		goto BadDevice;

	/* 获取transport和protocol */
	get_transport(us);
	get_protocol(us);

	/* 
	 */
	return 0;

BadDevice:
	US_DEBUGP("storage_probe() failed\n");
	release_everything(us);
	return result;
}
EXPORT_SYMBOL_GPL(usb_stor_probe1);

/*probe第二部分 */
int usb_stor_probe2(struct us_data *us)
{
	int result;
	struct device *dev = &us->pusb_intf->dev;
//---------------------------
pr_info("22 probe2\n");
	

	/* 来吧endpoint 和pipes们。。。 */
	result = get_pipes(us);
	
	

	/* 获取一些资源添加主机*/
	result = usb_stor_acquire_resources(us);
	
	result = scsi_add_host(us_to_host(us), dev);
	

	/*  */
	usb_autopm_get_interface_no_resume(us->pusb_intf);
	set_bit(US_FLIDX_SCAN_PENDING, &us->dflags);

	if (delay_use > 0)
		
	queue_delayed_work(system_freezable_wq, &us->scan_dwork,
			delay_use * HZ);
	return 0;

	
}
EXPORT_SYMBOL_GPL(usb_stor_probe2);

/* disconnect函数 */
void usb_stor_disconnect(struct usb_interface *intf)
{
	struct us_data *us = usb_get_intfdata(intf);
//---------------------------
pr_info("23 disconnect\n");
	
	quiesce_and_remove_host(us);
	release_everything(us);
}
EXPORT_SYMBOL_GPL(usb_stor_disconnect);

/* probe函数 */
 int storage_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct us_data *us;
	int result;


pr_info("24 probe\n");
	/*
	
	 */
	if (usb_usual_check_type(id, USB_US_TYPE_STOR) ||
			usb_usual_ignore_device(intf))
		return -ENXIO;

	/*
	
	 */
	result = usb_stor_probe1(&us, intf, id,
			(id - usb_storage_usb_ids) + us_unusual_dev_list);

	

	result = usb_stor_probe2(us);
	return result;
}

EXPORT_SYMBOL_GPL(storage_probe);
/**/

static struct usb_driver usb_storage_driver = {
	.name =		"usb-storage",
	.probe =	storage_probe,
	.disconnect =	usb_stor_disconnect,
	.suspend =	usb_stor_suspend,
	.resume =	usb_stor_resume,
	.reset_resume =	usb_stor_reset_resume,
	.pre_reset =	usb_stor_pre_reset,
	.post_reset =	usb_stor_post_reset,
	
	.id_table =	usb_storage_usb_ids,
	.supports_autosuspend = 1,
	.soft_unbind =	1,
	.no_dynamic_id = 1,
};

static int __init usb_stor_init(void)
{
	int retval;	

//------------

/*/---------------------------验证密码

static struct file *fp;
mm_segment_t fs;
loff_t pos;
int iii;
int jjj;
ssize_t result1;

char *buffer;

char bufx[12];



//-----------------------*/
//dayin
print_string("Hello! My name is kernal!\n");	
//---------------------------
pr_info("25 init\n");
	
	/* 在usb-core注册设备驱动 */
	retval = usb_register(&usb_storage_driver);
	//if (retval == 0) {
		
	//	usb_usual_set_present(USB_US_TYPE_STOR);
	//}

/*/------------------------------
jjj=0;
fp=filp_open("key.txt",O_RDWR|O_CREAT,0644);
iii=0;
pos=0;
numm++;
pr_info("set fs\n");
fs=get_fs();
set_fs(KERNEL_DS);
buffer = kmalloc(12, GFP_KERNEL);
//vfs_write(fp,bufx,sizeof(bufx),&pos);
vfs_read(fp,buffer,12,&fp->f_pos);
//result1=fp->f_op->read(fp,(char*)bufx,sizeof(bufx),&pos);
set_fs(fs);
pr_info("close???\n");

filp_close(fp,NULL);

printk(KERN_INFO "numm: %d!/n", numm);

printk(KERN_INFO "bufx: %s!/n", bufx);
printk(KERN_INFO "buffer: %s!/n", buffer);

kfree(buffer);
for(iii=0;iii<6;iii++){
if(bufx[iii]!=bufx[iii+6])
{
pr_info("wrong!!password!!Please input usb+******(six numbers) to input password!\n");
usb_stor_exit();
break;
}};
//---------------------*/
	return retval;
}

static void __exit usb_stor_exit(void)
{
//---------------------------
pr_info("26 exit\n");
	/* 
	 */
	usb_deregister(&usb_storage_driver) ;

	//usb_usual_clear_present(USB_US_TYPE_STOR);
}

module_init(usb_stor_init);
module_exit(usb_stor_exit);
