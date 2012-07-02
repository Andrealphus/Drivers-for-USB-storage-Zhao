
static void __exit usb_stor_exit(void)
{
	usb_deregister(&usb_storage_driver) ;

}


static int __init usb_stor_init(void)
{
	int retval;	


print_string("Hello! My name is kernal!\n");	

	retval = usb_register(&usb_storage_driver);

	return retval;
}
void usb_stor_disconnect(struct usb_interface *intf)
{
	struct us_data *us = usb_get_intfdata(intf);
	quiesce_and_remove_host(us);
	release_everything(us);
}
EXPORT_SYMBOL_GPL(usb_stor_disconnect);

static void quiesce_and_remove_host(struct us_data *us)
{
	struct Scsi_Host *host = us_to_host(us);
	//Prevent SCSI-scanning
	cancel_delayed_work_sync(&us->scan_dwork);
	
        scsi_remove_host(host);
	/*阻止任何对USB的命令*/
	scsi_lock(host);
	set_bit(US_FLIDX_DISCONNECTING, &us->dflags);
	scsi_unlock(host);
	wake_up(&us->delay_wait);
}

static void release_everything(struct us_data *us)
{

	usb_stor_release_resources(us);
	dissociate_dev(us);

	scsi_host_put(us_to_host(us));//对这个 host 的引用计数减 1,如果该 host 的引用计数达到 0 了,那么将释放其对应的 Scsi_Host 数据结构所占的空间

}

/* 与USB设备分离 */
static void dissociate_dev(struct us_data *us)
{

	/* Free the buffers */
	kfree(us->cr);//us->cr
	usb_free_coherent(us->pusb_dev, US_IOBUF_SIZE, us->iobuf, us->iobuf_dma);//us->iobuf

	/* 从接口处移除设备*/
	usb_set_intfdata(us->pusb_intf, NULL);//usb_set_intfdata 来令 us->pusb_intf 的所对应的设备的 driver_data 指向了 us，现在指向NULL
}

static void usb_stor_release_resources(struct us_data *us)
{

	complete(&us->cmnd_ready);
	if (us->ctl_thread)
		kthread_stop(us->ctl_thread);

	usb_free_urb(us->current_urb);
}

module_init(usb_stor_init);
module_exit(usb_stor_exit);
