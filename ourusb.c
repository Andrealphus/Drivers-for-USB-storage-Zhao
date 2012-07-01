

#include<asm/uaccess.h>
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

#include "sysin/usb.h"
#include "sysin/scsiglue.h"
#include "sysin/transport.h"
#include "sysin/protocol.h"
#include "sysin/debug.h"
#include "sysin/initializers.h"

#include "sysin/sierra_ms.h"
#include "sysin/option_ms.h"

 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("19xiaozu");
 MODULE_DESCRIPTION("19 team usb storage driver for linux");
//传参 第二个宏描述
static unsigned int delay_new=1;
module_param(delay_new,uint,S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(delay_new,"seconds to delay before using a new device");

static char quirks[128];
module_param_string(quirks,quirks,sizeof(quirks),S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(quirks,"supplemental list of device IDS and their quirks");

static int usb_stor_control_thread(void * __us);

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
#	include "sysin/unusual_devs.h" 
	{ }		/* Terminating entry */
};

#undef UNUSUAL_DEV
#undef COMPLIANT_DEV
#undef USUAL_DEV


#ifdef CONFIG_PM	/* Minimal support for suspend and resume */
//----LINE 133 support for suspend and resume
int our_suspend(struct usb_interface *iface, pm_message_t message){return 0;};
EXPORT_SYMBOL_GPL(our_suspend);
//----LINE 154 resume
int our_resume(struct usb_interface *iface){return 0;};
EXPORT_SYMBOL_GPL(our_resume);
//----LINE 169 
int our_reset_resume(struct usb_interface *iface){return 0;}
EXPORT_SYMBOL_GPL(our_reset_resume);
//---- 以上似乎是编译内核时可选项，可暂停恢复？？？先不实现

//以下函数当usb端口重置时候被调用，从这个或是从另一个调用
int our_pre_reset(struct usb_interface *iface){return 0;}
EXPORT_SYMBOL_GPL(our_pre_reset);
int our_post_reset(struct usb_interface *iface){return 0;}
EXPORT_SYMBOL_GPL(our_post_reset);




//usb_stor_sg_tablisize

static unsigned int usb_stor_sg_tablisize(struct usb_interface *intf){
struct usb_device *usb_dev= interface_to_usbdev(intf);
printk(KERN_ALERT"20 USB STOR SG TABLISIZE\n");

return usb_dev->bus->sg_tablesize;
}


EXPORT_SYMBOL_GPL(usb_stor_sg_tablisize);


//associate_dev
static int associate_dev(struct us_data *us,struct usb_interface *intf)
{
pr_info("8 associate_dev\n");
//初始化一些  us  与intf 对应
us->pusb_dev=interface_to_usbdev(intf);
us->pusb_intf=intf;
us->ifnum=intf->cur_altsetting->desc.bInterfaceNumber;
//存储我们的数据进入intf               n
usb_set_intfdata(intf,us);
//control request 
us->cr=kmalloc(sizeof(*us->cr),GFP_KERNEL);
us->iobuf = usb_alloc_coherent(us->pusb_dev, US_IOBUF_SIZE,
			GFP_KERNEL, &us->iobuf_dma);
return 0;
}

EXPORT_SYMBOL_GPL(associate_dev);
//get device infomation
static int get_device_info(struct us_data *us,const struct usb_device_id *id,struct us_unusual_dev *unusual_dev)
{
struct usb_device *dev=us->pusb_dev;
struct usb_interface_descriptor *idesc = &us->pusb_intf->cur_altsetting->desc;
//struct device *pdev=&us->pusb_intf->dev;
pr_info("10 get device info\n");
us->unusual_dev=unusual_dev;
us->subclass = (unusual_dev->useProtocol == USB_SC_DEVICE) ?
			idesc->bInterfaceSubClass :
			unusual_dev->useProtocol;
us->protocol = (unusual_dev->useTransport == USB_PR_DEVICE) ?
			idesc->bInterfaceProtocol :
			unusual_dev->useTransport;
us->fflags = USB_US_ORIG_FLAGS(id->driver_info);
adjust_quirks(us);
	/*
	 * This flag is only needed when we're in high-speed, so let's
	 * disable it if we're in full-speed
	 */
	if (dev->speed != USB_SPEED_HIGH)
		us->fflags &= ~US_FL_GO_SLOW;
return 0;

}


EXPORT_SYMBOL_GPL(get_device_info);



static void adjust_quirks(struct us_data *us)
{
	char *p;
	u16 vid = le16_to_cpu(us->pusb_dev->descriptor.idVendor);
	u16 pid = le16_to_cpu(us->pusb_dev->descriptor.idProduct);
	unsigned f = 0;
	unsigned int mask = (US_FL_SANE_SENSE | US_FL_BAD_SENSE |
			US_FL_FIX_CAPACITY |
			US_FL_CAPACITY_HEURISTICS | US_FL_IGNORE_DEVICE |
			US_FL_NOT_LOCKABLE | US_FL_MAX_SECTORS_64 |
			US_FL_CAPACITY_OK | US_FL_IGNORE_RESIDUE |
			US_FL_SINGLE_LUN | US_FL_NO_WP_DETECT |
			US_FL_NO_READ_DISC_INFO | US_FL_NO_READ_CAPACITY_16 |
			US_FL_INITIAL_READ10);
//---------------------------
pr_info("9 adjust_quirks\n");
	p = quirks;
	while (*p) {
		/* Each entry consists of VID:PID:flags */
		if (vid == simple_strtoul(p, &p, 16) &&
				*p == ':' &&
				pid == simple_strtoul(p+1, &p, 16) &&
				*p == ':')
			break;

		/* Move forward to the next entry */
		while (*p) {
			if (*p++ == ',')
				break;
		}
	}
	if (!*p)	/* No match */
		return;

	/* Collect the flags */
	while (*++p && *p != ',') {
		switch (TOLOWER(*p)) {
		case 'a':
			f |= US_FL_SANE_SENSE;
			break;
		case 'b':
			f |= US_FL_BAD_SENSE;
			break;
		case 'c':
			f |= US_FL_FIX_CAPACITY;
			break;
		case 'd':
			f |= US_FL_NO_READ_DISC_INFO;
			break;
		case 'e':
			f |= US_FL_NO_READ_CAPACITY_16;
			break;
		case 'h':
			f |= US_FL_CAPACITY_HEURISTICS;
			break;
		case 'i':
			f |= US_FL_IGNORE_DEVICE;
			break;
		case 'l':
			f |= US_FL_NOT_LOCKABLE;
			break;
		case 'm':
			f |= US_FL_MAX_SECTORS_64;
			break;
		case 'n':
			f |= US_FL_INITIAL_READ10;
			break;
		case 'o':
			f |= US_FL_CAPACITY_OK;
			break;
		case 'r':
			f |= US_FL_IGNORE_RESIDUE;
			break;
		case 's':
			f |= US_FL_SINGLE_LUN;
			break;
		case 'w':
			f |= US_FL_NO_WP_DETECT;
			break;
		/* Ignore unrecognized flag characters */
		}
	}
	us->fflags = (us->fflags & ~mask) | f;
}




EXPORT_SYMBOL_GPL(adjust_quirks);


/* Get the transport settings */
static void get_transport(struct us_data *us)
{
//---------------------------
pr_info("11 get transport\n");
	switch (us->protocol) {
	case USB_PR_CB:
		us->transport_name = "Control/Bulk";
		us->transport = usb_stor_CB_transport;
		us->transport_reset = usb_stor_CB_reset;
		us->max_lun = 7;
		break;

	case USB_PR_CBI:
		us->transport_name = "Control/Bulk/Interrupt";
		us->transport = usb_stor_CB_transport;
		us->transport_reset = usb_stor_CB_reset;
		us->max_lun = 7;
		break;

	case USB_PR_BULK:
		us->transport_name = "Bulk";
		us->transport = usb_stor_Bulk_transport;
		us->transport_reset = usb_stor_Bulk_reset;
		break;
	}
}



EXPORT_SYMBOL_GPL(get_transport);



/* Get the protocol settings */
static void get_protocol(struct us_data *us)
{
//---------------------------
pr_info("12 get protocol\n");
	switch (us->subclass) {
	case USB_SC_RBC:
		us->protocol_name = "Reduced Block Commands (RBC)";
		us->proto_handler = usb_stor_transparent_scsi_command;
		break;

	case USB_SC_8020:
		us->protocol_name = "8020i";
		us->proto_handler = usb_stor_pad12_command;
		us->max_lun = 0;
		break;

	case USB_SC_QIC:
		us->protocol_name = "QIC-157";
		us->proto_handler = usb_stor_pad12_command;
		us->max_lun = 0;
		break;

	case USB_SC_8070:
		us->protocol_name = "8070i";
		us->proto_handler = usb_stor_pad12_command;
		us->max_lun = 0;
		break;

	case USB_SC_SCSI:
		us->protocol_name = "Transparent SCSI";
		us->proto_handler = usb_stor_transparent_scsi_command;
		break;

	case USB_SC_UFI:
		us->protocol_name = "Uniform Floppy Interface (UFI)";
		us->proto_handler = usb_stor_ufi_command;
		break;
	}
}

EXPORT_SYMBOL_GPL(get_protocol);


/* Get the pipe settings */
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
	 * Find the first endpoint of each type we need.
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt-in is OK (necessary for CBI protocol).
	 * We will ignore any others.
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

	/* Calculate and store the pipe values */
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

EXPORT_SYMBOL_GPL(get_pipes);

/* Initialize all the dynamic resources we need */
static int usb_stor_acquire_resources(struct us_data *us)
{
	int p;
	struct task_struct *th;
//---------------------------
pr_info("14 usb stor acquire resources\n");


	us->current_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!us->current_urb) {
		US_DEBUGP("URB allocation failed\n");
		return -ENOMEM;
	}

	/* Just before we start our control thread, initialize
	 * the device if it needs initialization */
	if (us->unusual_dev->initFunction) {
		p = us->unusual_dev->initFunction(us);
		if (p)
			return p;
	}

	/* Start up our control thread */
	th = kthread_run(usb_stor_control_thread, us, "usb-storage");
	
	us->ctl_thread = th;

	return 0;
}

EXPORT_SYMBOL_GPL(usb_stor_acquire_resources);

//仔细研读
static int usb_stor_control_thread(void * __us)
{

	struct us_data *us = (struct us_data *)__us;
	struct Scsi_Host *host = us_to_host(us);
//---------------------------
pr_info("7 usb stor control thread\n");

	for(;;) {
		US_DEBUGP("*** thread sleeping.\n");
		if (wait_for_completion_interruptible(&us->cmnd_ready))
			break;

		US_DEBUGP("*** thread awakened.\n");

		/* lock the device pointers */
		mutex_lock(&(us->dev_mutex));

		/* lock access to the state */
		scsi_lock(host);

		/* When we are called with no command pending, we're done */
		if (us->srb == NULL) {
			scsi_unlock(host);
			mutex_unlock(&us->dev_mutex);
			US_DEBUGP("-- exiting\n");
			break;
		}

		/* has the command timed out *already* ? */
		if (test_bit(US_FLIDX_TIMED_OUT, &us->dflags)) {
			us->srb->result = DID_ABORT << 16;
			goto SkipForAbort;
		}

		scsi_unlock(host);

		/* reject the command if the direction indicator 
		 * is UNKNOWN
		 */
		if (us->srb->sc_data_direction == DMA_BIDIRECTIONAL) {
			US_DEBUGP("UNKNOWN data direction\n");
			us->srb->result = DID_ERROR << 16;
		}

		/* reject if target != 0 or if LUN is higher than
		 * the maximum known LUN
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

		/* Handle those devices which need us to fake 
		 * their inquiry data */
		else if ((us->srb->cmnd[0] == INQUIRY) &&
			    (us->fflags & US_FL_FIX_INQUIRY)) {
			unsigned char data_ptr[36] = {
			    0x00, 0x80, 0x02, 0x02,
			    0x1F, 0x00, 0x00, 0x00};

			US_DEBUGP("Faking INQUIRY command\n");
			fill_inquiry_response(us, data_ptr, 36);
			us->srb->result = SAM_STAT_GOOD;
		}

		/* we've got a command, let's do it! */
		else {
			US_DEBUG(usb_stor_show_command(us->srb));
			us->proto_handler(us->srb, us);
			usb_mark_last_busy(us->pusb_dev);
		}

		/* lock access to the state */
		scsi_lock(host);

		/* indicate that the command is done */
		if (us->srb->result != DID_ABORT << 16) {
			US_DEBUGP("scsi cmd done, result=0x%x\n", 
				   us->srb->result);
			us->srb->scsi_done(us->srb);
		} else {
SkipForAbort:
			US_DEBUGP("scsi command aborted\n");
		}

		/* If an abort request was received we need to signal that
		 * the abort has finished.  The proper test for this is
		 * the TIMED_OUT flag, not srb->result == DID_ABORT, because
		 * the timeout might have occurred after the command had
		 * already completed with a different result code. */
		if (test_bit(US_FLIDX_TIMED_OUT, &us->dflags)) {
			complete(&(us->notify));

			/* Allow USB transfers to resume */
			clear_bit(US_FLIDX_ABORTING, &us->dflags);
			clear_bit(US_FLIDX_TIMED_OUT, &us->dflags);
		}

		/* finished working on this command */
		us->srb = NULL;
		scsi_unlock(host);

		/* unlock the device pointers */
		mutex_unlock(&us->dev_mutex);
	} /* for (;;) */

	/* Wait until we are told to stop */
	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (kthread_should_stop())
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);
	return 0;
}	


EXPORT_SYMBOL_GPL(usb_stor_control_thread);



/* Delayed-work routine to carry out SCSI-device scanning */
static void usb_stor_scan_dwork(struct work_struct *work)
{
	struct us_data *us = container_of(work, struct us_data,
			scan_dwork.work);
	struct device *dev = &us->pusb_intf->dev;
//---------------------------
pr_info("19 usb stor scan dwork\n");

	dev_dbg(dev, "starting scan\n");

	/* For bulk-only devices, determine the max LUN value */
	if (us->protocol == USB_PR_BULK && !(us->fflags & US_FL_SINGLE_LUN)) {
		mutex_lock(&us->dev_mutex);
		us->max_lun = usb_stor_Bulk_max_lun(us);
		mutex_unlock(&us->dev_mutex);
	}
	scsi_scan_host(us_to_host(us));
	dev_dbg(dev, "scan complete\n");

	/* Should we unbind if no devices were detected? */

	usb_autopm_put_interface(us->pusb_intf);
	clear_bit(US_FLIDX_SCAN_PENDING, &us->dflags);
}


EXPORT_SYMBOL_GPL(usb_stor_scan_dwork);


//probe书中有教
static int our_probe(struct usb_interface *intf,
			 const struct usb_device_id *id){
struct us_data *us;
int result;
//device
struct device *dev;
//scsihost
struct Scsi_Host *host;
//检测id是否符合 和intf
struct us_unusual_dev *unusual_dev;

unusual_dev=(id - usb_storage_usb_ids) + us_unusual_dev_list;
if(usb_usual_check_type(id,USB_US_TYPE_STOR) || usb_usual_ignore_device(intf))
return -ENXIO;

printk(KERN_ALERT "probe usb   usb  detected!\n");
//分配个host

host = scsi_host_alloc(&usb_stor_host_template,sizeof(*us));
if(!host){
dev_warn(&intf->dev,"fail to allocate the scsi host\n");
return -ENOMEM;
}
//host中的一些初始化
host->max_cmd_len = 16;
host-> sg_tablesize = usb_stor_sg_tablesize(intf);
us= host_to_us(host);//us 作为host中 的us
//分内存？
memset(us,0,sizeof(struct us_data));
mutex_init(&(us->dev_mutex));
init_completion(&us->cmnd_ready);
init_completion(&(us->notify));
init_waitqueue_head(&us->delay_wait);
INIT_DELAYED_WORK(&us->scan_dwork,usb_stor_scan_dwork);

result = associate_dev(us,intf);
if(result)
         goto Bad;
result = get_device_info(us,id,unusual_dev);
if(result)
         goto Bad;
//transport protocol
get_transport(us);
get_protocol(us);

if(!us->transport ||!us->proto_handler){
result=-ENXIO;
goto Bad;
}
printk(KERN_ALERT"Transport: %s\n",us->transport_name);
printk(KERN_ALERT"Protocol: %s\n",us->transport_name);
dev = &us->pusb_intf->dev;
//设置max lun
if(us->fflags & US_FL_SINGLE_LUN)
us->max_lun =0;

//endpoint get pipe
result = get_pipes(us);
if(result)
goto Bad;
//如果u盘前十个指令错误，重置
if (us->fflags & US_FL_INITIAL_READ10)
		set_bit(US_FLIDX_REDO_READ10, &us->dflags);
//申请子资源，添加进host
result=usb_stor_acquire_sesources(us);
if(result)
goto Bad;
snprintf(us->scsi_name,sizeof(us->scsi_name),"our-usb-storage%s",dev_name(&us->pusb_intf->dev));
result= scsi_add_host(us_to_host(us),dev);
if(result){
printk(KERN_ALERT"UNable to add the host\n");
goto Bad;
}
//scsi设备延时探测
usb_autopm_get_interface_no_resume(us->pusb_intf);
set_bit(US_FLIDX_SCAN_PENDING,&us->dflags);
if(delay_use>0)
          dev_dbg(dev,"waiting for device before scanning\n");
queue_delayed_work(system_freezable_wq,&us->scan_dwork,delay_use * HZ);


return 0;
Bad:

 printk(KERN_ALERT "probe false!\n");
release_everything(us);
return result;
   }

EXPORT_SYMBOL_GPL(our_probe);

void our_disconnect(struct usb_interface *intf){
struct us_data *us = usb_get_intfdata(intf);
//---------------------------
pr_info("23 disconnect\n");
	US_DEBUGP("storage_disconnect() called\n");
	quiesce_and_remove_host(us);
	release_everything(us);
}

EXPORT_SYMBOL_GPL(our_disconnect);
static void quiesce_and_remove_host(struct us_data *us)
{
	struct Scsi_Host *host = us_to_host(us);
//---------------------------
pr_info("17 quiesce and remove host\n");
	/* If the device is really gone, cut short reset delays */
	if (us->pusb_dev->state == USB_STATE_NOTATTACHED) {
		set_bit(US_FLIDX_DISCONNECTING, &us->dflags);
		wake_up(&us->delay_wait);
	}

	/* Prevent SCSI scanning (if it hasn't started yet)
	 * or wait for the SCSI-scanning routine to stop.
	 */
	cancel_delayed_work_sync(&us->scan_dwork);

	/* Balance autopm calls if scanning was cancelled */
	if (test_bit(US_FLIDX_SCAN_PENDING, &us->dflags))
		usb_autopm_put_interface_no_suspend(us->pusb_intf);

	/* Removing the host will perform an orderly shutdown: caches
	 * synchronized, disks spun down, etc.
	 */
	scsi_remove_host(host);

	/* Prevent any new commands from being accepted and cut short
	 * reset delays.
	 */
	scsi_lock(host);
	set_bit(US_FLIDX_DISCONNECTING, &us->dflags);
	scsi_unlock(host);
	wake_up(&us->delay_wait);
}

EXPORT_SYMBOL_GPL(quiesce_and_remove_host);
static void release_everything(struct us_data *us)
{
//---------------------------
pr_info("18  release_everything\n");
	usb_stor_release_resources(us);
	dissociate_dev(us);

	/* Drop our reference to the host; the SCSI core will free it
	 * (and "us" along with it) when the refcount becomes 0. */
	scsi_host_put(us_to_host(us));
}



EXPORT_SYMBOL_GPL(release_everything);

static void usb_stor_release_resources(struct us_data *us)
{

//---------------------------
pr_info("15 usb stor release resources\n");
	US_DEBUGP("-- %s\n", __func__);

	/* Tell the control thread to exit.  The SCSI host must
	 * already have been removed and the DISCONNECTING flag set
	 * so that we won't accept any more commands.
	 */
	US_DEBUGP("-- sending exit command to thread\n");
	complete(&us->cmnd_ready);
	if (us->ctl_thread)
		kthread_stop(us->ctl_thread);

	/* Call the destructor routine, if it exists */
	if (us->extra_destructor) {
		US_DEBUGP("-- calling extra_destructor()\n");
		us->extra_destructor(us->extra);
	}

	/* Free the extra data and the URB */
	kfree(us->extra);
	usb_free_urb(us->current_urb);
}



EXPORT_SYMBOL_GPL(usb_stor_release_resources);
static void dissociate_dev(struct us_data *us)
{
//---------------------------
pr_info("16 dissociate dev\n");

	US_DEBUGP("-- %s\n", __func__);

	/* Free the buffers */
	kfree(us->cr);
	usb_free_coherent(us->pusb_dev, US_IOBUF_SIZE, us->iobuf, us->iobuf_dma);

	/* Remove our private data from the interface */
	usb_set_intfdata(us->pusb_intf, NULL);
}



EXPORT_SYMBOL_GPL(dissociate_dev);




//初始化以及向usb core 注册
static struct usb_driver usb_storage_driver={
//书中所写可先实现
.name="ourusb-storage",
.probe=our_probe,
.disconnect=our_disconnect,
.id_table=usb_storage_usb_ids,

//源自内核文件
.suspend= our_suspend,
.resume= our_resume,
.reset_resume=our_reset_resume,
.pre_reset= our_pre_reset,
.post_reset= our_post_reset,
.supports_autosuspend=1,
.soft_unbind=1,
.no_dynamic_id=1,
};
 static int __init ourusb_init(void)
{
int result;
 printk(KERN_ALERT "ourusb initializing!\n");
result=usb_register(&usb_storage_driver);
if(result == 0){
 printk(KERN_ALERT "our usb support registered.\n");
usb_usual_set_present(USB_US_TYPE_STOR);//这句话目前不懂什么意思。。以后研究
}
 return result;
 }	

static void __exit ourusb_exit(void)
 {

printk(KERN_ALERT "our usb exit() called\n");
usb_deregister(&usb_storage_driver);

usb_usual_clear_present(USB_US_TYPE_STOR);//ok目前依旧不懂。
 }

module_init(ourusb_init);
 module_exit(ourusb_exit);

