
#include <linux/module.h>    
#include <linux/kernel.h> 
#include <linux/fs.h>	   // Inode and File types
#include <linux/cdev.h>    // Character Device Types and functions.
#include <linux/types.h>
#include <asm/uaccess.h>   // Copy to/from user space   
#include <linux/init.h>  
#include <linux/slab.h>
#include <linux/device.h>  // Device Creation / Destruction functions
#include <linux/gpio.h>
#include <linux/delay.h>
#include<linux/moduleparam.h> // Passing parameters to modules through insmod
#include <asm/msr.h>
#include <linux/interrupt.h>

#define gpio_1_Mux 30
#define gpio_0_Mux 31
#define trigger_pin 14
#define echo_pin 15
#define DEVICE_NAME                 "usonic"  // device name to be created and registered

struct timer_list my_timer;

/* per device structure */
struct usonic_dev {
	struct cdev cdev;   /* The cdev structure */
    int cms; 
	
} *usonic_devp;

static dev_t usonic_dev_number;      /* Allotted device number */
struct class *usonic_dev_class;          /* Tie with the device model */
static struct device *usonic_dev_device;

/// timer handler
void Start_Usonic (unsigned long data){
	
	struct timer_list *timer = (struct timer_list*)data;
	
	gpio_set_value(trigger_pin, 1); //14
	udelay(10);
	gpio_set_value(trigger_pin, 0); //14
	
	timer->expires = jiffies+(15);
	add_timer(timer);
	
	return ;
}
/// IRQ Handler
unsigned long time_rise, time_fall, time_diff;
bool entry =0;
static irq_handler_t echo_handler(int irq, void *dev_id, struct pt_regs *regs)
{	
 	 if(entry == 0){
	 	rdtscl(time_rise);
		entry =1;
		irq_set_irq_type(irq,IRQF_TRIGGER_FALLING);
		}
	 else{
		rdtscl(time_fall);
		time_diff = time_fall - time_rise;
		usonic_devp->cms = (time_diff / (58*400));
		entry =0;
		irq_set_irq_type(irq,IRQF_TRIGGER_RISING);
		}
	 	
	return (irq_handler_t)IRQ_HANDLED;
}

/*
* Open usonic driver
*/

int usonic_driver_open(struct inode *inode, struct file *file)
{
	struct usonic_dev *usonic_devp;
	int ret;
	unsigned int echo_irq = 0;
	/* Get the per-device structure that contains this cdev */
	usonic_devp = container_of(inode->i_cdev, struct usonic_dev, cdev);

	file->private_data = usonic_devp;
	printk("\n device is openning \n");

	ret = gpio_request(30, "gpio_1_Mux");    // interrupt input mux for echo
	if(ret < 0)
	{
		printk("Error Requesting GPIO1_Mux.\n");
		return -1;
	}

	ret = gpio_request(31, "gpio_0_Mux"); //  mux for trigger
	if(ret < 0)
	{
		printk("Error Requesting GPIO1_Mux.\n");
		return -1;
	}

	ret = gpio_request(0, "gpio_0_puMux"); //MUX for Trigger
	if(ret < 0)
	{
		printk("Error Requesting GPIO2_puMux.\n");

		return -1;
	}
	
	ret = gpio_request(1, "gpio_1_puMux");  //MUX for echo
	if(ret < 0)
	{
		printk("Error Requesting GPIO2_puMux.\n");

		return -1;
	}

	ret = gpio_request(14, "trigger_pin"); // Trigger op
	if(ret < 0)
	{
		printk("Error Requesting IRQ 14.\n");
		return -1;
	}

	ret = gpio_request(15, "echo_pin"); // Echo ip
	if(ret < 0)
	{
		printk("Error Requesting IRQ 15.\n");
		return -1;
	}



	ret = gpio_direction_output(gpio_0_Mux, 0); //31
	if(ret < 0)
	{
		printk("Error Setting GPIO_0_Mux output.\n");
	}
	ret = gpio_direction_output(0, 0);
	if(ret < 0)
	{
		printk("Error Setting GPIO_2_puMux output.\n");
	}
	
	ret = gpio_direction_output(trigger_pin , 0); //14
	if(ret < 0)
	{
		printk("Error Setting trigger Pin Output.\n");
	}

	ret = gpio_direction_output(gpio_1_Mux, 0); //30
	if(ret < 0)
	{
		printk("Error Setting GPIO_2_Mux output.\n");
	}

	ret = gpio_direction_output(1, 0);
	if(ret < 0)
	{
		printk("Error Setting GPIO_2_puMux output.\n");
	}

	ret = gpio_direction_input(echo_pin); //15
	if(ret < 0)
	{
		printk("Error Setting Echo Pin Input.\n");
	}



	gpio_set_value_cansleep(gpio_0_Mux, 0); //31 Set GPIO 0 Mux to 0
	gpio_set_value_cansleep(0, 0); // Set GPIO 0 pullup Mux to 0
	
	gpio_set_value_cansleep(gpio_1_Mux, 0); // Set GPIO 1 Mux to 0
	gpio_set_value_cansleep(1, 0); // Set GPIO 1 pullup Mux to 0


	gpio_set_value(trigger_pin, 0); //14 set trigger to 0
	
	echo_irq = gpio_to_irq(echo_pin);   // associate irq to echo pin
	
    // Initialize timer
	my_timer.function = Start_Usonic;
	my_timer.expires = jiffies + 15;
	my_timer.data = (unsigned long)&my_timer; // for restart
	init_timer(&my_timer);
	
	add_timer(&my_timer);

	ret = request_irq(echo_irq, (irq_handler_t)echo_handler, IRQF_TRIGGER_RISING, "Echo_Dev", (void *)(echo_irq));
	if(ret < 0)
	{
		printk("Error requesting IRQ: %d\n", ret);
	}	
	
	return 0;
}

/*
 * Release usonic driver
 */
int usonic_driver_release(struct inode *inode, struct file *file)
{
	//struct usonic_dev *usonic_devp = file->private_data;
	unsigned int echo_irq =0;
	gpio_free(gpio_0_Mux);
	gpio_free(0);
	gpio_free(trigger_pin);

	gpio_free(gpio_1_Mux);
	gpio_free(1);
	
	
	echo_irq = gpio_to_irq(echo_pin);
	free_irq(echo_irq, (void *)(echo_irq));
	
	gpio_free(echo_pin);	
	del_timer(&my_timer);
	printk("\n sonic device is closing\n");
	
	return 0;
}

/*
 * Read to usonic driver
 */
ssize_t usonic_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	int echo_dist = 0,ret =0;
	struct usonic_dev *usonic_devp = file->private_data;
	echo_dist = usonic_devp->cms;
	ret = copy_to_user((int *)buf,&echo_dist, sizeof(echo_dist)); 
	if(ret != 0)	{	
		printk("could not copy to user, ret= %d",ret);	
		return -EINVAL;
	 	}
	return 0;

}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations usonic_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= usonic_driver_open,        /* Open method */
    .release	= usonic_driver_release,     /* Release method */
    .read		= usonic_driver_read,        /* Read method */
};

static int __init init_usonic(void)
{
	int ret =0;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&usonic_dev_number, 0, 1, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	usonic_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	usonic_devp = kmalloc(sizeof(struct usonic_dev), GFP_KERNEL);
		
	if (!usonic_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	/* Request I/O region */
	//sprintf(usonic_devp->name, DEVICE_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&usonic_devp->cdev, &usonic_fops);
	usonic_devp->cdev.owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&usonic_devp->cdev, (usonic_dev_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	usonic_dev_device = device_create(usonic_dev_class, NULL, MKDEV(MAJOR(usonic_dev_number), 0), NULL, DEVICE_NAME);	
	
	
	return 0;
}

static void __exit exit_usonic(void)
{

	/* Release the major number */
	unregister_chrdev_region((usonic_dev_number), 1);

	/* Destroy device */
	device_destroy (usonic_dev_class, MKDEV(MAJOR(usonic_dev_number), 0));
	cdev_del(&usonic_devp->cdev);
	kfree(usonic_devp);
	
	/* Destroy driver_class */
	class_destroy(usonic_dev_class);
}

module_init(init_usonic);
module_exit(exit_usonic);

MODULE_LICENSE("GPL v2");
