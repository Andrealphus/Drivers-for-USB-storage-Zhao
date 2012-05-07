/** patch submitted by Vivian Bregier <Vivian.Bregier@imag.fr>
	 */
	UNUSUAL_DEV(  0x03eb, 0x2002, 0x0100, 0x0100,
	        "ATMEL",
	        "SND1 Storage",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE),
	 
	/** Reported by Rodolfo Quesada <rquesada@roqz.net> */
	UNUSUAL_DEV(  0x03ee, 0x6906, 0x0003, 0x0003,
	        "VIA Technologies Inc.",
	        "Mitsumi multi cardreader",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE ),
	 
	UNUSUAL_DEV(  0x03f0, 0x0107, 0x0200, 0x0200,
	        "HP",
	        "CD-Writer+",
	        USB_SC_8070, USB_PR_CB, NULL, 0),
	 
	/** Reported by Ben Efros <ben@pc-doctor.com> */
	UNUSUAL_DEV(  0x03f0, 0x070c, 0x0000, 0x0000,
	        "HP",
	        "Personal Media Drive",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_SANE_SENSE ),
	 
	/** Reported by Grant Grundler <grundler@parisc-linux.org>
	 * HP r707 camera in "Disk" mode with 2.00.23 or 2.00.24 firmware.
	 */
	UNUSUAL_DEV(  0x03f0, 0x4002, 0x0001, 0x0001,
	        "HP",
	        "PhotoSmart R707",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_FIX_CAPACITY),
	 
	/** Reported by Sebastian Kapfer <sebastian_kapfer@gmx.net>
	 * and Olaf Hering <olh@suse.de> (different bcd's, same vendor/product)
	 * for USB floppies that need the SINGLE_LUN enforcement.
	 */
	UNUSUAL_DEV(  0x0409, 0x0040, 0x0000, 0x9999,
	        "NEC",
	        "NEC USB UF000x",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_SINGLE_LUN ),
	 
	/** Patch submitted by Mihnea-Costin Grigore <mihnea@zulu.ro> */
	UNUSUAL_DEV(  0x040d, 0x6205, 0x0003, 0x0003,
	        "VIA Technologies Inc.",
	        "USB 2.0 Card Reader",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE ),
	 


//New additor is palced here
	UNUSUAL_DEV( 0x0951, 0x1624, 0x0001, 0xffff,
		"Kingston",
		"DataTraveler G2",
		US_SC_SCSI, US_PR_BULK, NULL, 
		US_FL_FIX_INQUIRY),
//placed termianted

	UNUSUAL_DEV(  0x0411, 0x001c, 0x0113, 0x0113,
	        "Buffalo",
	        "DUB-P40G HDD",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_FIX_INQUIRY ),
	 
	/** Submitted by Ernestas Vaiciukevicius <ernisv@gmail.com> */
	UNUSUAL_DEV(  0x0419, 0x0100, 0x0100, 0x0100,
	        "Samsung Info. Systems America, Inc.",
	        "MP3 Player",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE ),
	 
	/** Reported by Orgad Shaneh <orgads@gmail.com> */
	UNUSUAL_DEV(  0x0419, 0xaace, 0x0100, 0x0100,
	        "Samsung", "MP3 Player",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE ),
	 
	/** Reported by Christian Leber <christian@leber.de> */
	UNUSUAL_DEV(  0x0419, 0xaaf5, 0x0100, 0x0100,
	        "TrekStor",
	        "i.Beat 115 2.0",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE | US_FL_NOT_LOCKABLE ),
	 
	/** Reported by Stefan Werner <dustbln@gmx.de> */
	UNUSUAL_DEV(  0x0419, 0xaaf6, 0x0100, 0x0100,
	        "TrekStor",
	        "i.Beat Joy 2.0",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE ),
	 
	/** Reported by Pete Zaitcev <zaitcev@redhat.com>, bz#176584 */
	UNUSUAL_DEV(  0x0420, 0x0001, 0x0100, 0x0100,
	        "GENERIC", "MP3 PLAYER", /** MyMusix PD-205 on the outside. */
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_IGNORE_RESIDUE ),
	
	 
	#ifdef NO_SDDR09
	UNUSUAL_DEV(  0x0436, 0x0005, 0x0100, 0x0100,
	        "Microtech",
	        "CameraMate",
	        USB_SC_SCSI, USB_PR_CB, NULL,
	        US_FL_SINGLE_LUN ),
	#endif
	/** Patch submitted by Daniel Drake <dsd@gentoo.org>
	 * Device reports nonsense bInterfaceProtocol 6 when connected over USB2 */
	UNUSUAL_DEV(  0x0451, 0x5416, 0x0100, 0x0100,
	        "Neuros Audio",
	        "USB 2.0 HD 2.5",
	        USB_SC_DEVICE, USB_PR_BULK, NULL,
	        US_FL_NEED_OVERRIDE ),
UNUSUAL_DEV(  0x05e3, 0x0701, 0x0000, 0xffff,
0790	        "Genesys Logic",
0791	        "USB to IDE Optical",
0792	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
0793	        US_FL_GO_SLOW | US_FL_MAX_SECTORS_64 | US_FL_IGNORE_RESIDUE ),
0794	 
0795	UNUSUAL_DEV(  0x05e3, 0x0702, 0x0000, 0xffff,
0796	        "Genesys Logic",
0797	        "USB to IDE Disk",
0798	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
0799	        US_FL_GO_SLOW | US_FL_MAX_SECTORS_64 | US_FL_IGNORE_RESIDUE ),
	 
	/** Reported by Ben Efros <ben@pc-doctor.com> */
	UNUSUAL_DEV(  0x05e3, 0x0723, 0x9451, 0x9451,
	        "Genesys Logic",
	        "USB to SATA",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_SANE_SENSE ),
	 
/** Reported by Hanno Boeck <hanno@gmx.de>
	 * Taken from the Lycoris Kernel */
	UNUSUAL_DEV(  0x0636, 0x0003, 0x0000, 0x9999,
	        "Vivitar",
      "Vivicam 35Xx",
	        USB_SC_SCSI, USB_PR_BULK, NULL,
	        US_FL_FIX_INQUIRY ),
	 
	UNUSUAL_DEV(  0x0644, 0x0000, 0x0100, 0x0100,
	        "TEAC",
	        "Floppy Drive",
        USB_SC_UFI, USB_PR_CB, NULL, 0 ),
	 
	/** Reported by Darsen Lu <darsen@micro.ee.nthu.edu.tw> */
	UNUSUAL_DEV( 0x066f, 0x8000, 0x0001, 0x0001,
	        "SigmaTel",
	        "USBMSC Audio Player",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_FIX_CAPACITY ),
 
/** Reported by Daniel Kukula <daniel.kuku@gmail.com> */
UNUSUAL_DEV( 0x067b, 0x1063, 0x0100, 0x0100,
	        "Prolific Technology, Inc.",
        "Prolific Storage Gadget",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_BAD_SENSE ),
	 
	/** Reported by Rogerio Brito <rbrito@ime.usp.br> */
	UNUSUAL_DEV( 0x067b, 0x2317, 0x0001, 0x001,
	        "Prolific Technology, Inc.",
	        "Mass Storage Device",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_NOT_LOCKABLE ),
	 
	/** Reported by Richard -=[]=- <micro_flyer@hotmail.com> */
	/** Change to bcdDeviceMin (0x0100 to 0x0001) reported by
	 * Thomas Bartosik <tbartdev@gmx-topmail.de> */
	UNUSUAL_DEV( 0x067b, 0x2507, 0x0001, 0x0100,
	        "Prolific Technology Inc.",
	        "Mass Storage Device",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_FIX_CAPACITY | US_FL_GO_SLOW ),
	 
	/** Reported by Alex Butcher <alex.butcher@assursys.co.uk> */
	UNUSUAL_DEV( 0x067b, 0x3507, 0x0001, 0x0101,
	        "Prolific Technology Inc.",
	        "ATAPI-6 Bridge Controller",
	        USB_SC_DEVICE, USB_PR_DEVICE, NULL,
	        US_FL_FIX_CAPACITY | US_FL_GO_SLOW ),
	 
	

/** Control/Bulk transport for all SubClass values */
	USUAL_DEV(USB_SC_RBC, USB_PR_CB, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_8020, USB_PR_CB, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_QIC, USB_PR_CB, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_UFI, USB_PR_CB, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_8070, USB_PR_CB, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_SCSI, USB_PR_CB, USB_US_TYPE_STOR),
	 
	/** Control/Bulk/Interrupt transport for all SubClass values */
	USUAL_DEV(USB_SC_RBC, USB_PR_CBI, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_8020, USB_PR_CBI, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_QIC, USB_PR_CBI, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_UFI, USB_PR_CBI, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_8070, USB_PR_CBI, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_SCSI, USB_PR_CBI, USB_US_TYPE_STOR),
	 
	/** Bulk-only transport for all SubClass values */
	USUAL_DEV(USB_SC_RBC, USB_PR_BULK, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_8020, USB_PR_BULK, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_QIC, USB_PR_BULK, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_UFI, USB_PR_BULK, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_8070, USB_PR_BULK, USB_US_TYPE_STOR),
	USUAL_DEV(USB_SC_SCSI, USB_PR_BULK, 0),


