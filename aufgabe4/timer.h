#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/jiffies.h>
#include <linux/string.h>
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



#define MINOR_START 0
#define TIMER_DEVICE_COUNT 2
#define BUFFER_SIZE 128

//scaling stuff
#define S_TIMESCALE HZ
#define MS_TIMESCALE S_TIMESCALE/1000
#define JIFFIES_TIMESCALE 1
