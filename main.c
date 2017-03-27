#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <signal.h>

void spi_send(char array[] , int fd, struct spi_ioc_transfer xfer , char* writeBuf);

int main(int argc, char* argv[])
{
	int fd =0, fd2= 0, fdk =0,ret=0;
	unsigned char mode = 0;
	unsigned char lsb = 0; 
	int echo_dist=0;
	char readBuf[2];
	char writeBuf[2];
	struct spi_ioc_transfer xfer;
	void siginthandler(int);
	int safe_distance = 5; //default safe distance = 5
	int toggle = 0;
	char led_arr[][8] = { 
	{0x81 , 0x42 , 0x24 , 0x18 , 0x18 , 0x24 , 0x42 , 0x81 } ,  // x
	{0x0,0x0,0x0,0xFF,0xFF,0x0,0x0,0x0} ,  // 	 		2 bars
	{0x0,0x0,0xFF,0xFF,0xFF,0xFF,0x0,0x0} ,  //	       4 bars
	{0x0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0} ,  //     6 bars
	{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF} , //   8 bars
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}  //    off 
	};
	
	signal(SIGINT, siginthandler);
	
	if(argc ==2){
		safe_distance = atoi(argv[1]);
		printf("Safe Distance set to %d \n" , safe_distance);
	}
	
	fd2 = open("/sys/class/gpio/export", O_WRONLY);
	write(fd2, "42", 2);
	close(fd2); 
	
	// Set GPIO Direction
	fd2 = open("/sys/class/gpio/gpio42/direction", O_WRONLY);
	write(fd2, "out", 3);
	close(fd2);
	
	// Set Value
	fd2 = open("/sys/class/gpio/gpio42/value", O_WRONLY);
	write(fd2, "0", 1);



	fd2 = open("/sys/class/gpio/export", O_WRONLY);
	write(fd2, "43", 2);
	close(fd2); 
	
	// Set GPIO Direction
	fd2 = open("/sys/class/gpio/gpio43/direction", O_WRONLY);
	write(fd2, "out", 3);
	close(fd2);
	
	// Set Value
	fd2 = open("/sys/class/gpio/gpio43/value", O_WRONLY);
	write(fd2, "0", 1);
    close(fd2);
	
	fd2 = open("/sys/class/gpio/export", O_WRONLY);
	write(fd2, "55", 2);
	close(fd2); 
	
	// Set GPIO Direction
	fd2 = open("/sys/class/gpio/gpio55/direction", O_WRONLY);
	write(fd2, "out", 3);
	close(fd2);
	
	// Set Value
	fd2 = open("/sys/class/gpio/gpio55/value", O_WRONLY);
	write(fd2, "0", 1);
	close(fd2);

	fd = open("/dev/spidev1.0", O_RDWR);

	mode |= SPI_MODE_0;
	
	// Mode Settings could include:
	// 		SPI_LOOP		0
	//		SPI_MODE_0		1
	//		SPI_MODE_1		2
	//		SPI_MODE_2		3
	//		SPI_LSB_FIRST	4
	//		SPI_CS_HIGH		5
	//		SPI_3WIRE		6
	//		SPI_NO_CS		7		
	
	if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
	{
		perror("SPI Set Mode Failed");
		close(fd);
		return -1;
	}
	
	if(ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb) < 0)
	{
		perror("SPI Set LSB Failed");
		close(fd);
		return -1;
	}
/*
if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bpw) < 0)
	{
		perror("SPI Set LSB Failed");
		close(fd);
		return -1;
	}

if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &ms) < 0)
	{
		perror("SPI Set LSB Failed");
		close(fd);
		return -1;
	}
*/	
	// Setup Commands could include:
	//		SPI_IOC_RD_MODE
	//		SPI_IOC_WR_MODE
	//		SPI_IOC_RD_LSB_FIRST
	//		SPI_IOC_WR_LSB_FIRST
	//		SPI_IOC_RD_BITS_PER_WORD
	//		SPI_IOC_WR_BITS_PER_WORD
	//		SPI_IOC_RD_MAX_SPEED_HZ
	//		SPI_IOC_WR_MAX_SPEED_HZ


	// Setup a Write Transfer
	
	// Setup Transaction 
	xfer.tx_buf = (unsigned long)writeBuf;
	xfer.rx_buf = (unsigned long)NULL;
	xfer.len = 2; // Bytes to send
	xfer.cs_change = 0;
	xfer.delay_usecs = 0;
	xfer.speed_hz = 10000000;
	xfer.bits_per_word = 8;


	writeBuf[1] = 0x00; // Step 1
	writeBuf[0] = 0x09; 
	
	// Send Message
	if(ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");

	writeBuf[1] = 0x03; // Step 2
	writeBuf[0] = 0x0A; 
	
	// Send Message
	if(ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");

	writeBuf[1] = 0x07; // Step 3
	writeBuf[0] = 0x0B; 
	
	// Send Message
	if(ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");

	writeBuf[1] = 0x01; // Step 4
	writeBuf[0] = 0x0C; 
	
	// Send Message
	if(ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");

   // Testing mode off
	writeBuf[1] = 0x00; // Step 5
	writeBuf[0] = 0x0F; 
	// Send Message
	if(ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");
	
	fdk =  open("/dev/usonic", O_RDONLY);    
	
	if(fdk < 0)
	{
		printf("Error: could not open USONIC device.\n");
		return -1;
	}
	
	while(1)
	{
		ret = read(fdk,&echo_dist, sizeof(int));
		printf("Distance = %d\n", echo_dist);
		if(ret < 0)
		{
			//printf("Error: could not read dev.\n");
			 printf("Recd: %d\n", echo_dist);
			 perror("Error: ");
			return -1;
		}
		
		if(echo_dist<=safe_distance){
			if(!toggle){
				spi_send(led_arr[0], fd , xfer,  writeBuf);
				toggle = !toggle;
			}
			else if(toggle){
				spi_send(led_arr[5], fd , xfer,  writeBuf);
				toggle = !toggle;			
			}
		}
		else if((echo_dist>(safe_distance)) && (echo_dist<=(safe_distance+10))){	
			spi_send(led_arr[1], fd , xfer,  writeBuf);
			toggle = 0;
		}
		else if((echo_dist>(safe_distance+10)) && (echo_dist<=(safe_distance+30))){	
			spi_send(led_arr[2], fd , xfer,  writeBuf);
			toggle = 0;
		}
		else if((echo_dist>(safe_distance+30)) && (echo_dist<=(safe_distance+40))){
			spi_send(led_arr[3], fd , xfer,  writeBuf);
			toggle = 0;
		}
		else{
			spi_send(led_arr[4], fd , xfer,  writeBuf);
			toggle = 0;
		}
		usleep(120 * 1000);   // poll for distance every 120ms
	}
	//close(fd);
}
 
void siginthandler(int signum)
{	
    printf("Caught signal %d, ending program...\n", signum);
    exit(1);//_exit() closes all associated open fds
}

void spi_send(char array[] ,int fd, struct spi_ioc_transfer xfer , char* writeBuf){
	
	int i = 0;	
	for(i = 1 ; i<= 8 ; i++){
	
	writeBuf[1] = array[i-1]; // Step 1
	writeBuf[0] = (char)i;  //address
	
	if(ioctl(fd, SPI_IOC_MESSAGE(1), &xfer) < 0)
		perror("SPI Message Send Failed");
	}
}


