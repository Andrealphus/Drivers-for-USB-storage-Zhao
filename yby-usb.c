
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
//为模块添加一些信息
MODULE_AUTHOR("hahahaha");
MODULE_DESCRIPTION("USB Mass Storage driver for Linux");
MODULE_LICENSE("GPL");
//设置延迟时间
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
	{ }		/* Terminating entry */
};

#undef UNUSUAL_DEV
#undef COMPLIANT_DEV
#undef USUAL_DEV


#ifdef CONFIG_PM	/* Minimal support for suspend and resume */

int usb_stor_suspend(struct usb_interface *iface, pm_message_t message)
{
	struct us_data *us = usb_get_intfdata(iface);

//----------------------------
pr_info("1 usb usb_stor_suspend\n");
	/* Wait until no command is running */
	mutex_lock(&us->dev_mutex);

	US_DEBUGP("%s\n", __func__);
	if (us->suspend_resume_hook)
		(us->suspend_resume_hook)(us, US_SUSPEND);

	/* When runtime PM is working, we'll set a flag to indicate
	 * whether we should autoresume when a SCSI request arrives. */

	mutex_unlock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_suspend);

int usb_stor_resume(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);
//------------------------------
pr_info("2 usb stor resume\n");
	mutex_lock(&us->dev_mutex);

	US_DEBUGP("%s\n", __func__);
	if (us->suspend_resume_hook)
		(us->suspend_resume_hook)(us, US_RESUME);

	mutex_unlock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_resume);

int usb_stor_reset_resume(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);
//------------------------
pr_info("3 usb stor reset resume\n");
	US_DEBUGP("%s\n", __func__);

	/* Report the reset to the SCSI core */
	usb_stor_report_bus_reset(us);

	/* FIXME: Notify the subdrivers that they need to reinitialize
	 * the device */
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_reset_resume);

#endif /* CONFIG_PM */

/*
 * The next two routines get called just before and just after
 * a USB port reset, whether from this driver or a different one.
 */

int usb_stor_pre_reset(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);
//--------------------------
pr_info("4 usb stor pre reset\n");
	US_DEBUGP("%s\n", __func__);

	/* Make sure no command runs during the reset */
	mutex_lock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_pre_reset);

int usb_stor_post_reset(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);
//-------------------------------
pr_info("5 usb stor post reset\n");
	US_DEBUGP("%s\n", __func__);

	/* Report the reset to the SCSI core */
	usb_stor_report_bus_reset(us);

	/* FIXME: Notify the subdrivers that they need to reinitialize
	 * the device */

	mutex_unlock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_post_reset);

/*
 * fill_inquiry_response takes an unsigned char array (which must
 * be at least 36 characters) and populates the vendor name,
 * product name, and revision fields. Then the array is copied
 * into the SCSI command's response buffer (oddly enough
 * called request_buffer). data_len contains the length of the
 * data array, which again must be at least 36.
 */

void fill_inquiry_response(struct us_data *us, unsigned char *data,
		unsigned int data_len)
{
//------------------------------
pr_info("6 fill_inquiry response\n");
	if (data_len<36) // You lose.
		return;

	memset(data+8, ' ', 28);
	if(data[0]&0x20) { /* USB device currently not connected. Return
			      peripheral qualifier 001b ("...however, the
			      physical device is not currently connected
			      to this logical unit") and leave vendor and
			      product identification empty. ("If the target
			      does store some of the INQUIRY data on the
			      device, it may return zeros or ASCII spaces 
			      (20h) in those fields until the data is
			      available from the device."). */
	} else {
		u16 bcdDevice = le16_to_cpu(us->pusb_dev->descriptor.bcdDevice);
		int n;

		n = strlen(us->unusual_dev->vendorName);
		memcpy(data+8, us->unusual_dev->vendorName, min(8, n));
		n = strlen(us->unusual_dev->productName);
		memcpy(data+16, us->unusual_dev->productName, min(16, n));

		data[32] = 0x30 + ((bcdDevice>>12) & 0x0F);
		data[33] = 0x30 + ((bcdDevice>>8) & 0x0F);
		data[34] = 0x30 + ((bcdDevice>>4) & 0x0F);
		data[35] = 0x30 + ((bcdDevice) & 0x0F);
	}

	usb_stor_set_xfer_buf(data, data_len, us->srb);
}
EXPORT_SYMBOL_GPL(fill_inquiry_response);

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
//print sth on terminal
static void print_string(char *str)
{
    struct tty_struct *my_tty;
    my_tty = current->signal->tty;
    if (my_tty != NULL)
    {
          my_tty->driver->ops->write(my_tty,str,strlen(str));
      
    }
}
/***********************************************************************
 * Device probing and disconnecting
 ***********************************************************************/

/*调用于probe，将us与intr联系起来 */
static int associate_dev(struct us_data *us, struct usb_interface *intf)
{
//---------------------------
pr_info("8 associate dev \n");
	US_DEBUGP("-- %s\n", __func__);

	/* us中的参数各种赋值 */
	us->pusb_dev = interface_to_usbdev(intf);

//point of usb device  这里因为usb-core需要usb_dev类型，所以进行了类型转换，木有实际意义
	us->pusb_intf = intf;

//接口编号 intf number就这样赋值了
	us->ifnum = intf->cur_altsetting->desc.bInterfaceNumber;

//其实也就是从intf中提取些设备信息打印出来。
	US_DEBUGP("Vendor: 0x%04x, Product: 0x%04x, Revision: 0x%04x\n",
			le16_to_cpu(us->pusb_dev->descriptor.idVendor),
			le16_to_cpu(us->pusb_dev->descriptor.idProduct),
			le16_to_cpu(us->pusb_dev->descriptor.bcdDevice));
	US_DEBUGP("Interface Subclass: 0x%02x, Protocol: 0x%02x\n",
			intf->cur_altsetting->desc.bInterfaceSubClass,
			intf->cur_altsetting->desc.bInterfaceProtocol);

	/* 最终该函数让intf->dev->driver_data赋值为us */

	usb_set_intfdata(intf, us);

	/*  为us->cr,us->buf申请内存*/
	us->cr = kmalloc(sizeof(*us->cr), GFP_KERNEL);
	if (!us->cr) {
		US_DEBUGP("usb_ctrlrequest allocation failed\n");
		return -ENOMEM;
	}
       //这里来看，dma是支持的
	us->iobuf = usb_alloc_coherent(us->pusb_dev, US_IOBUF_SIZE,
			GFP_KERNEL, &us->iobuf_dma);
	if (!us->iobuf) {
		US_DEBUGP("I/O buffer allocation failed\n");
		return -ENOMEM;
	}
	return 0;
}

/* Works only for digits and letters, but small and fast */
#define TOLOWER(x) ((x) | 0x20)

/* Adjust device flags based on the "quirks=" module parameter */
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

/* Get the unusual_devs entries and the string descriptors */
static int get_device_info(struct us_data *us, const struct usb_device_id *id,
		struct us_unusual_dev *unusual_dev)
{
	struct usb_device *dev = us->pusb_dev;
//接口描述符
	struct usb_interface_descriptor *idesc =
		&us->pusb_intf->cur_altsetting->desc;
	struct device *pdev = &us->pusb_intf->dev;

//---------------------------
pr_info("10  get device info\n");
	/* Store the entries */
	us->unusual_dev = unusual_dev;
	us->subclass = (unusual_dev->useProtocol == USB_SC_DEVICE) ?
			idesc->bInterfaceSubClass :
			unusual_dev->useProtocol;
	us->protocol = (unusual_dev->useTransport == USB_PR_DEVICE) ?
			idesc->bInterfaceProtocol :
			unusual_dev->useTransport;
	us->fflags = USB_US_ORIG_FLAGS(id->driver_info);
	adjust_quirks(us);

	if (us->fflags & US_FL_IGNORE_DEVICE) {
		dev_info(pdev, "device ignored\n");
		return -ENODEV;
	}

	/*
	 * This flag is only needed when we're in high-speed, so let's
	 * disable it if we're in full-speed
	 */
	if (dev->speed != USB_SPEED_HIGH)
		us->fflags &= ~US_FL_GO_SLOW;

	if (us->fflags)
		dev_info(pdev, "Quirks match for vid %04x pid %04x: %lx\n",
				le16_to_cpu(dev->descriptor.idVendor),
				le16_to_cpu(dev->descriptor.idProduct),
				us->fflags);

	/* Log a message if a non-generic unusual_dev entry contains an
	 * unnecessary subclass or protocol override.  This may stimulate
	 * reports from users that will help us remove unneeded entries
	 * from the unusual_devs.h table.
	 */
	if (id->idVendor || id->idProduct) {
		static const char *msgs[3] = {
			"an unneeded SubClass entry",
			"an unneeded Protocol entry",
			"unneeded SubClass and Protocol entries"};
		struct usb_device_descriptor *ddesc = &dev->descriptor;
		int msg = -1;

		if (unusual_dev->useProtocol != USB_SC_DEVICE &&
			us->subclass == idesc->bInterfaceSubClass)
			msg += 1;
		if (unusual_dev->useTransport != USB_PR_DEVICE &&
			us->protocol == idesc->bInterfaceProtocol)
			msg += 2;
		if (msg >= 0 && !(us->fflags & US_FL_NEED_OVERRIDE))
			dev_notice(pdev, "This device "
					"(%04x,%04x,%04x S %02x P %02x)"
					" has %s in unusual_devs.h (kernel"
					" %s)\n"
					"   Please send a copy of this message to "
					"<linux-usb@vger.kernel.org> and "
					"<usb-storage@lists.one-eyed-alien.net>\n",
					le16_to_cpu(ddesc->idVendor),
					le16_to_cpu(ddesc->idProduct),
					le16_to_cpu(ddesc->bcdDevice),
					idesc->bInterfaceSubClass,
					idesc->bInterfaceProtocol,
					msgs[msg],
					utsname()->release);
	}

	return 0;
}

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
	if (IS_ERR(th)) {
		dev_warn(&us->pusb_intf->dev,
				"Unable to start control thread\n");
		return PTR_ERR(th);
	}
	us->ctl_thread = th;

	return 0;
}

/* Release all our dynamic resources */
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

/* Dissociate from the USB device */
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

/* First stage of disconnect processing: stop SCSI scanning,
 * remove the host, and stop accepting new commands
 */
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

/* Second stage of disconnect processing: deallocate all resources */
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

static unsigned int usb_stor_sg_tablesize(struct usb_interface *intf)
{
	struct usb_device *usb_dev = interface_to_usbdev(intf);
//---------------------------
printk(KERN_ALERT"20 usb stor sg tablesize\n");


	if (usb_dev->bus->sg_tablesize) {
		return usb_dev->bus->sg_tablesize;
	}
	return SG_ALL;
}

/* First part of general USB mass-storage probing */
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
我们把u盘看作一个scsi设备，于是我们需要一个scsihost，我们申请host并让其工作，需要调用三个函数scsi_host_alloc(),scsi_add_host(),scsi_scan_host()
	 */
//scsi_host_alloc()一旦调用，则为scsi_host申请空间，于是把us数据结构和host联系起来。自此可以使用内联函数us_to_host(),host_to_us()
	host = scsi_host_alloc(&usb_stor_host_template, sizeof(*us));
	if (!host) {
		dev_warn(&intf->dev,
				"Unable to allocate the scsi host\n");
		return -ENOMEM;
	}

	/*
	 * Allow 16-byte CDBs and thus > 2TB
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

	/*该函数将us和接口intf联系起来      */
	result = associate_dev(us, intf);
	if (result)
		goto BadDevice;

	/* 该函数旨在于让驱动知道设备的一些信息，描述 */
	result = get_device_info(us, id, unusual_dev);
	if (result)
		goto BadDevice;

	/* Get standard transport and protocol settings */
	get_transport(us);
	get_protocol(us);

	/* Give the caller a chance to fill in specialized transport
	 * or protocol settings.
	 */
	return 0;

BadDevice:
	US_DEBUGP("storage_probe() failed\n");
	release_everything(us);
	return result;
}
EXPORT_SYMBOL_GPL(usb_stor_probe1);

/* Second part of general USB mass-storage probing */
int usb_stor_probe2(struct us_data *us)
{
	int result;
	struct device *dev = &us->pusb_intf->dev;
//---------------------------
pr_info("22 probe2\n");
	/* Make sure the transport and protocol have both been set */
	if (!us->transport || !us->proto_handler) {
		result = -ENXIO;
		goto BadDevice;
	}
	US_DEBUGP("Transport: %s\n", us->transport_name);
	US_DEBUGP("Protocol: %s\n", us->protocol_name);

	/* fix for single-lun devices */
	if (us->fflags & US_FL_SINGLE_LUN)
		us->max_lun = 0;

	/* Find the endpoints and calculate pipe values */
	result = get_pipes(us);
	if (result)
		goto BadDevice;

	/*
	 * If the device returns invalid data for the first READ(10)
	 * command, indicate the command should be retried.
	 */
	if (us->fflags & US_FL_INITIAL_READ10)
		set_bit(US_FLIDX_REDO_READ10, &us->dflags);

	/* Acquire all the other resources and add the host */
	result = usb_stor_acquire_resources(us);
	if (result)
		goto BadDevice;
	snprintf(us->scsi_name, sizeof(us->scsi_name), "usb-storage %s",
					dev_name(&us->pusb_intf->dev));
	result = scsi_add_host(us_to_host(us), dev);
	if (result) {
		dev_warn(dev,
				"Unable to add the scsi host\n");
		goto BadDevice;
	}

	/* Submit the delayed_work for SCSI-device scanning */
	usb_autopm_get_interface_no_resume(us->pusb_intf);
	set_bit(US_FLIDX_SCAN_PENDING, &us->dflags);

	if (delay_use > 0)
		dev_dbg(dev, "waiting for device to settle before scanning\n");
	queue_delayed_work(system_freezable_wq, &us->scan_dwork,
			delay_use * HZ);
	return 0;

	/* We come here if there are any problems */
BadDevice:
	US_DEBUGP("storage_probe() failed\n");
	release_everything(us);
	return result;
}
EXPORT_SYMBOL_GPL(usb_stor_probe2);

/* Handle a USB mass-storage disconnect */
void usb_stor_disconnect(struct usb_interface *intf)
{
	struct us_data *us = usb_get_intfdata(intf);
//---------------------------
pr_info("23 disconnect\n");
	US_DEBUGP("storage_disconnect() called\n");
	quiesce_and_remove_host(us);
	release_everything(us);
}
EXPORT_SYMBOL_GPL(usb_stor_disconnect);

/* The main probe routine for standard devices */
 int storage_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct us_data *us;//详见usb.h
	int result;


pr_info("24 probe\n");
	/*
	 该函数为了保证设备是属于usb—storage的。
	 */
	if (usb_usual_check_type(id, USB_US_TYPE_STOR) ||
			usb_usual_ignore_device(intf))
		return -ENXIO;

	/*
	 * Call the general probe procedures.
	 *
	 * The unusual_dev_list array is parallel to the usb_storage_usb_ids
	 * table, so we use the index of the id entry to find the
	 * corresponding unusual_devs entry.
	 */
	result = usb_stor_probe1(&us, intf, id,
			(id - usb_storage_usb_ids) + us_unusual_dev_list);
	if (result)
		return result;

	/* No special transport or protocol settings in the main module */

	result = usb_stor_probe2(us);
	return result;
}

EXPORT_SYMBOL_GPL(storage_probe);
/***********************************************************************
 * Initialization and registration
 ***********************************************************************/
//将usb.h中usb_driver，对应。。。我们不用管，只是知道有什么就行。详见linux/usb.h
static struct usb_driver usb_storage_driver = {
	.name =		"usb-storage",//名字
	.probe =	storage_probe,//探测函数
	.disconnect =	usb_stor_disconnect,//注销函数
	.suspend =	usb_stor_suspend,
	.resume =	usb_stor_resume,
	.reset_resume =	usb_stor_reset_resume,
	.pre_reset =	usb_stor_pre_reset,
	.post_reset =	usb_stor_post_reset,
	.id_table =	usb_storage_usb_ids,//标明了该设备驱动支持哪些
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
	pr_info("----Initializing USB Mass Storage driver...\n");

	/* 向usb-core注册usb，driver */
	retval = usb_register(&usb_storage_driver);
	if (retval == 0) {
		pr_info("USB---- abcd ----  Mass Storage support registered.\n");
		usb_usual_set_present(USB_US_TYPE_STOR);
	}

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
	US_DEBUGP("usb_stor_exit() called\n");
//---------------------------
pr_info("26 exit\n");
//向usb-core注销该设备驱动
	US_DEBUGP("-- calling usb_deregister()\n");
	usb_deregister(&usb_storage_driver) ;

	usb_usual_clear_present(USB_US_TYPE_STOR);
}

module_init(usb_stor_init);
module_exit(usb_stor_exit);
