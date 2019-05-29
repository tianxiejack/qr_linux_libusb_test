#if 0
#include "libusb.h"

int dev_num;
struct device_descript dev_list[DEV_MIN];
libusb_context *ctx=NULL;

void dev_find_endpoints(libusb_device *dev,int vendorId,int productId) {
	libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		printf("failed to get device descriptor");
		return;
	}

	printf("Number of possible configurations: %d\n",
			(int) desc.bNumConfigurations);
	printf("Device Class: %d\n", (int) desc.bDeviceClass);
	printf("VendorID:%d \n", desc.idVendor);
	printf("ProductID: %d\n", desc.idProduct);
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	printf("Interfaces: %", (int) config->bNumInterfaces);
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;
	if(desc.idVendor==vendorId&&desc.idProduct==productId){
		dev_list[dev_num].dev=dev;
	for (int i = 0; i < (int) config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		printf("Number of alternate settings: %d\n", inter->num_altsetting);
		for (int j = 0; j < inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			printf("Interface Number: %d\n", (int) interdesc->bInterfaceNumber);
			printf("Interface class: %d\n", (int) interdesc->bInterfaceClass);
			printf("Number of endpoints: \n", (int) interdesc->bNumEndpoints);
			if(interdesc->bInterfaceClass==LIBUSB_CLASS_PRINTER ){
			for (int k = 0; k < (int) interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				printf("Descriptor Type:%d \n", (int) epdesc->bDescriptorType);
				printf("EP Address: %d\n", (int) epdesc->bEndpointAddress);
				int address=epdesc->bEndpointAddress;
				if(address>=128){
					dev_list[dev_num].in=address;
				}else{
					dev_list[dev_num].out=address;
				}
			}
			}
		}
	}
	 dev_num++;
	}
}
int dev_find(int vendor_id, int product_id) {

	libusb_device **devs;


	libusb_init(&ctx);
	int cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if (cnt < 0) {
		printf("Get Device Error \n");

		return 1;
	}
	printf("Devices in list. %d \n", cnt);

	int i = 0;
	for (i = 0; i < cnt; i++) {

		dev_find_endpoints(devs[i],vendor_id,product_id);
	}
	libusb_free_device_list(devs, 1);
	return cnt;
}

int dev_open(int dev_no) {
	dev_num=dev_no;
	if ( NULL != dev_list[dev_num].dev) {
		libusb_device_handle* handler;
		int result=libusb_open(dev_list[dev_num].dev,&handler);
		if(result!=0){
			printf("open error %d\n",result);
			return result;
		}
		dev_list[dev_num].handler=handler;
		if (libusb_kernel_driver_active(dev_list[dev_no].handler, 0) == 1) { //find out if kernel driver is attached
						printf("Kernel Driver Active.");

						if (libusb_detach_kernel_driver(dev_list[dev_no].handler, 0) == 0) //detach it
							printf("Kernel Driver Detached!");

					}
					int r = libusb_claim_interface(dev_list[dev_no].handler, 0); //claim interface 0 (the first) of device (mine had jsut 1)
					if (r < 0) {
						printf("Cannot Claim Interface\n");

						return 1;
					}
					printf("Claimed Interface \n");
	}
	return 0;

}

int dev_close(int dev_no) {
	int result=libusb_release_interface(dev_list[dev_no].handler, 0);
	libusb_close(dev_list[dev_no].handler); //close the device we opened
	dev_list[dev_no].handler=NULL;
	libusb_exit(ctx);
	ctx=NULL;
	return result;
}

int dev_write(int dev_no, unsigned char* ptr_out, int out_len) {

   int len_size;
	int size = libusb_bulk_transfer(dev_list[dev_no].handler, (dev_list[dev_no].out| LIBUSB_ENDPOINT_OUT),
			ptr_out, out_len, &len_size,
			dev_list[dev_no].outTime);

#ifdef DEBUG
	printf("Out buffer: %s\n", ptr_out);
	printf("Out length: %d;Success length: %d\n", out_len, len_size);
#endif


	if (len_size == out_len&&size==0) {
		return len_size;
	}


	return size;
}

int dev_read(int dev_no, unsigned char* ptr_in, int in_len) {
	int recv_len;
	int size=0;
	//size= libusb_bulk_transfer(dev_list[dev_no].handler, (dev_list[dev_no].in|LIBUSB_ENDPOINT_IN), ptr_in, in_len,
		//	&recv_len, dev_list[dev_no].inTime);
	size= libusb_bulk_transfer(dev_list[dev_no].handler, 1, ptr_in, in_len,
				&recv_len, dev_list[dev_no].inTime);

#ifdef DEBUG
	printf("\nReceived data length: %d\n", recv_len);
#endif

	if (recv_len < 0) {
		return RD_ER;
	}

	return recv_len;
}
int dev_isOpen(int dev_no){
	if(dev_list[dev_no].handler!=NULL){
		return 0;
	}
	return 1;

}
 int dev_isOnline(int dev_no){
	 unsigned char data[]="\x10\x04\x04";
	 	printf("data length %d",sizeof(data)/sizeof(char));
	 	int dataSend=dev_write(dev_no,data,sizeof(data)/sizeof(char));

	 	printf("sended size %d",dataSend);
	 	unsigned char reciever[1];

	 	int rev=dev_read(dev_no,reciever,1);
	 	printf("reviecer %d",rev);
	 	int i=0;
	 	for(;i<rev;i++){
	 		printf("data reviecer %d",reciever[i]);
	 	}

	 	return i>0?0:1;

 }
 void dev_setOption(int dev_no,int outTime,int inTime){
	 dev_list[dev_no].outTime=outTime;
	 dev_list[dev_no].inTime=inTime;

 }

#endif
