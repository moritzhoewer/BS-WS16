#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include<linux/jiffies.h>
#include<linux/string.h>
#include <asm/uaccess.h>

typedef enum {
	READY, LOAD, RUN, PAUSE
} state_t;

typedef enum {
	FORWARD, BACKWARD
} direction_t;

struct timer_device {
	state_t state;
	direction_t direction;
	u64 jiffies_start;
	u64 jiffies_pause_start;
	u64 jiffies_pause_total;
	unsigned int timescale;
	struct cdev cdev;
};

/*
 * Macros to help debugging
 */
#undef PDEBUG             /* undef it, just in case */
#ifdef DEBUG_MESSAGES
#  ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "timer: " fmt, ## args)
#  else
/* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

/*inline int min(int a, int b) {
 return (b < a) ? b : a;
 }*/

#define MIN(a, b) (b) < (a) ? (b) : (a)

MODULE_LICENSE("GPL");

#define MINOR_START 0
#define TIMER_DEVICE_COUNT 2
#define BUFFER_SIZE 128

static int timer_major;
static struct timer_device forward_timer;
static struct timer_device backward_timer;

/*
 * File Operations
 */
int timer_open(struct inode *inode, struct file *filp) {
	struct timer_device *dev = NULL;

	// get enclosign timer_device and link it to filep
	dev = container_of(inode->i_cdev, struct timer_device, cdev);
	filp->private_data = dev;
	PDEBUG("Device has direction %s\n", dev->direction == FORWARD ? "FORWARD" : "BACKWARD");

	return 0;
}

int timer_release(struct inode *inode, struct file *filp) {
	// Nothing to do
	PDEBUG("Releasing...\n");
	return 0;
}

ssize_t timer_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos) {
	int transferred = 0;
	struct timer_device *dev = filp->private_data;

	if (*f_pos != 0) { // second read command
		return 0;
	}

	PDEBUG("User requested data with count=%d\n", count);

	switch (dev->state) {
	case READY:
		transferred = MIN(16, count); // TODO: Magic Numbers
		if (copy_to_user(buf, "Timer is ready.\n", transferred)) {
			printk(KERN_WARNING "Cannot send data to user!\n");
			return -EFAULT;
		}
		break;
	case LOAD:
		transferred = MIN(17, count); // TODO: Magic Numbers
		if (copy_to_user(buf, "Timer is loaded.\n", transferred)) {
			printk(KERN_WARNING "Cannot send data to user!\n");
			return -EFAULT;
		}
		break;
	case PAUSE:
		transferred = MIN(17, count); // TODO: Magic Numbers
		if (copy_to_user(buf, "Timer is paused.\n", transferred)) {
			printk(KERN_WARNING "Cannot send data to user!\n");
			return -EFAULT;
		}
		break;
	case RUN:
		if (dev->direction == FORWARD) {
			u64 jif = get_jiffies_64();
			if (jif < dev->jiffies_start) {
				printk(KERN_WARNING "Whoops, timer encountered a wrap around when reading\n");

				transferred = MIN(46, count); // TODO: Magic Numbers
				if (copy_to_user(buf,
						"Sorry, there was a wrap around - resetting...\n",
						transferred)) {
					printk(KERN_WARNING "Cannot send data to user!\n");
					return -EFAULT;
				}
				dev->state = READY;
			} else {
				char my_buf[BUFFER_SIZE] = { 0 };
				u64 diff = ((jif - dev->jiffies_start) - dev->jiffies_pause_total);

				sprintf(my_buf, "%u\n", (unsigned int)diff / dev->timescale);
				transferred = MIN(strlen(my_buf), count);
				if (copy_to_user(buf, my_buf, transferred)) {
					printk(KERN_WARNING "Cannot send data to user!\n");
					return -EFAULT;
				}
			}
		} else { // Backward
			u64 jif = get_jiffies_64();
			if (jif > (dev->jiffies_start + dev->jiffies_pause_total)) {
				// done counting down
				transferred = MIN(2, count); // TODO: Magic Numbers
				if (copy_to_user(buf,"0\n",	transferred)) {
					printk(KERN_WARNING "Cannot send data to user!\n");
					return -EFAULT;
				}
			} else {
				char my_buf[BUFFER_SIZE] = { 0 };
				u64 diff = (dev->jiffies_start + dev->jiffies_pause_total - jif);

				sprintf(my_buf, "%u\n", (unsigned int)diff / dev->timescale);
				transferred = MIN(strlen(my_buf), count);
				if (copy_to_user(buf, my_buf, transferred)) {
					printk(KERN_WARNING "Cannot send data to user!\n");
					return -EFAULT;
				}
			}
		}
		break;
	}
	*f_pos = 10; // TODO: Magic Numbers
	return transferred;
}

ssize_t timer_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos) {
	struct timer_device *dev = filp->private_data;
	char my_buf[BUFFER_SIZE] = { 0 };
	int transferred = MIN(count, BUFFER_SIZE);
	int buffer_length;

	PDEBUG("User sent data with count=%d\n", count);

	if (copy_from_user(my_buf, buf, transferred)) {
		printk(KERN_WARNING "Cannot get data from user!\n");
		return -EFAULT;
	}

	buffer_length = strlen(my_buf);
	buffer_length--;
	if (buffer_length < 1) {
		printk(KERN_WARNING "Timer received empty command!\n");
		return 0;
	} else {
		my_buf[buffer_length] = 0;
	}

	// reset command
	if (my_buf[0] == 'r') {
		dev->state = READY;
	}
	// start command
	else if ((my_buf[0] == 's')
			&& ((dev->state == READY && dev->direction == FORWARD)
					|| (dev->state == LOAD && dev->direction == BACKWARD))) {
		// reset pause counter
		dev->jiffies_pause_total = 0;

		// if it's the forward counter, then we need to reset jiffies
		if (dev->direction == FORWARD) {
			dev->jiffies_start = 0;
		}
		dev->jiffies_start += get_jiffies_64();
		
		PDEBUG("Starting timer: %s with start=%llu and pause=%llu\n",
				dev->direction == FORWARD ? "FORWARD" : "BACKWARD",
						dev->jiffies_start, dev->jiffies_pause_total );

		dev->state = RUN;
	}
	// pause command
	else if (my_buf[0] == 'p' && dev->state == RUN) {
		dev->jiffies_pause_start = get_jiffies_64();
		dev->state = PAUSE;
	}
	// continue command
	else if (my_buf[0] == 'c' && dev->state == PAUSE) {
		u64 jif = get_jiffies_64();
		if (jif < dev->jiffies_pause_start) {
			printk(KERN_WARNING "Whoops, timer encountered a wrap around when unpausing\n");
			dev->state = READY;
		} else {
			dev->jiffies_pause_total += (jif - dev->jiffies_pause_start);

			PDEBUG("Resuming timer: %s with pause=%llu\n",
					dev->direction == FORWARD ? "FORWARD" : "BACKWARD",
							dev->jiffies_pause_total );

			dev->state = RUN;
		}
	}
	// load command
	else if (my_buf[0] == 'l' && dev->state == READY
			&& dev->direction == BACKWARD) {
		unsigned long timeval;
		int result = sscanf(my_buf + 1, "%lu", &timeval);
		if (result != 1) {
			printk(KERN_WARNING "Whoops, timer couldn't decode start time\n");
			dev->state = READY;
		} else {
			dev->jiffies_start = timeval * dev->timescale;
			dev->state = LOAD;
		}

	}
	// timescale command
	else if (my_buf[0] == 't') {
		if(buffer_length < 2){
			printk(KERN_WARNING "Timer recieved illegal time unit\n");
		}
		switch(my_buf[1]){
		case 'j':
			dev->timescale = 1;
			break;
		case 'm':
			dev->timescale = HZ / 1000;
			break;
		case 's':
			dev->timescale = HZ;
			break;
		default:
			printk(KERN_WARNING "Timer recieved illegal time unit\n");
		}

	}else {
		printk(KERN_WARNING "Timer recieved illegal command \"%s\"\n", my_buf);
	}

	return transferred;
}

struct file_operations timer_fops = {
		.owner = THIS_MODULE,
		.read = timer_read,
		.write = timer_write,
		.open = timer_open,
		.release = timer_release
};

/**
 * Module suff 
 */
static int timer_init(void) {
	int result;
	dev_t dev = 0;

	// request dynamic major number
	result = alloc_chrdev_region(&dev, MINOR_START, TIMER_DEVICE_COUNT, "timer");

	if (result < 0) {
		printk(KERN_WARNING "Cannot register Major number for timer!");
		return result;
	}
	timer_major = MAJOR(dev);
	PDEBUG("Registered with major number %d\n", timer_major);PDEBUG("HZ is %d\n", HZ);

	// init forward timer
	forward_timer.state = READY;
	forward_timer.direction = FORWARD;
	forward_timer.jiffies_start = 0;
	forward_timer.jiffies_pause_start = 0;
	forward_timer.jiffies_pause_total = 0;
	forward_timer.timescale = 1;
	cdev_init(&(forward_timer.cdev), &timer_fops);
	forward_timer.cdev.owner = THIS_MODULE;
	forward_timer.cdev.ops = &timer_fops;

	result = cdev_add(&(forward_timer.cdev), MKDEV(timer_major, MINOR_START), 1);
	if (result < 0) {
		printk(KERN_WARNING "Cannot register forward timer!");
		return result;
	}

	// init backward timer
	backward_timer.state = READY;
	backward_timer.direction = BACKWARD;
	backward_timer.jiffies_start = 0;
	backward_timer.jiffies_pause_start = 0;
	backward_timer.jiffies_pause_total = 0;
	backward_timer.timescale = 1;
	cdev_init(&(backward_timer.cdev), &timer_fops);
	backward_timer.cdev.owner = THIS_MODULE;
	backward_timer.cdev.ops = &timer_fops;

	result = cdev_add(&(backward_timer.cdev), MKDEV(timer_major, MINOR_START + 1),
			1);
	if (result < 0) {
		printk(KERN_WARNING "Cannot register backward timer!");
		return result;
	}

	return 0;
}

static void timer_exit(void) {
	dev_t devno = MKDEV(timer_major, MINOR_START);

	// remove devices
	cdev_del(&(forward_timer.cdev));
	cdev_del(&(backward_timer.cdev));

	// remove region
	unregister_chrdev_region(devno, TIMER_DEVICE_COUNT);

	PDEBUG("Unloaded timer\n");
}

module_init(timer_init)
module_exit(timer_exit)
