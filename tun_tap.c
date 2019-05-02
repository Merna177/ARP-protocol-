#include <stdio.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
/*commands used for creating tap device 
sudo openvpn --mktun --dev tap1
sudo ip addr add 10.0.0.1/24 dev tap1
sudo ip link set tap1 up
-----------------------------------------------
We have created a virtual network interface (tun0 in my case), accessible by the current user 
and set it’s address to 10.0.0.1 If we run ip link we observe that the link is always DOWN:
Since there is no program attached to this virtual interface 
the kernel treats it like an Ethernet adapter without a cable connected (NO-CARRIER).
*/
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
The TUN/TAP driver is a way for userspace applications to do networking. It creates a virtual network interface that behaves like a real one but every packet it receives gets forwarded to a userspace application. TUN adapters operate with layer 3 packets (such as IP packets) and TAP adapter works with layer 2 packets (like Ethernet frames). TUN is therefore useful for creating a point-to-point network tunnel while TAP can bridge two networks like a switch (but at a slightly higher overhead).
*/
static int tun_alloc(char *dev)
{ ///struct for linux network contain interface name and another set of variables like address and so on
    struct ifreq interface_data;
    int file_descriptor, err;
///first step open device 
    if( (file_descriptor = open("/dev/net/tun", O_RDWR)) < 0 ) {
        perror("Cannot open TUN/TAP dev\n");
        exit(1);
    }

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
     
    memset(&interface_data, 0, sizeof(interface_data));
    interface_data.ifr_flags = IFF_TAP | IFF_NO_PI;
    if( *dev ) {
/// copy name that i give it to the device in struct ifreq 
///IFNAMSIZ --> size of name that declared in struct 
        strncpy(interface_data.ifr_name, dev, IFNAMSIZ);
    }
///check if there is an error in creating interface
/*TUNSETIFF constant:
If a non-existent or empty interface name is specified, the kernel will create a new interface if the user has the CAP_NET_ADMIN permission (or is root).
*/
    if( (err = ioctl(file_descriptor, TUNSETIFF, (void *) &interface_data)) < 0 ){
        perror("ERR: Could not ioctl tun");
        close(file_descriptor);
        return err;
    }

    strcpy(dev, interface_data.ifr_name);
    return file_descriptor;
}
/*read and write to interface:
Reading and writing to the virtual interface is simple, we can use read() and write() like any file.
*/
///handle reading from interface
///return value (0,1) if 1 means successful read 
int read_from_interface(int file_descriptor,char *buffer, int buffer_length){
 int return_value = read(file_descriptor, buffer, buffer_length);
 if(return_value<0) return -1;
 else
  return return_value ;
}
int tun_write(int file_descriptor,char *buffer, int buffer_length)
{
    return write(file_descriptor, buffer, buffer_length);
}
int main(){

  char tun_name[IFNAMSIZ];
  strcpy(tun_name, "tap1");
  int file_descriptor = tun_alloc(tun_name);  
  if(file_descriptor < 0){
   printf("merna");
   printf("error in Allocating interface\n");
   return 0;
  }

while(1) {
  /* Now read data coming from the kernel */
  /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
  char buffer[1000];
  int read_value = read_from_interface(file_descriptor,buffer,sizeof(buffer));
  if(read_value == -1) {
    printf("error\n");
    close(file_descriptor);
    return 0;
  }
  printf("Read %d bytes from device %s %s\n", read_value, tun_name,buffer);
 
 }
  return 0;
}
