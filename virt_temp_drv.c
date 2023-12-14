#include <linux/module.h>       // Необходимо для всех модулей ядра
#include <linux/kernel.h>       // Необходимо для KERN_INFO
#include <linux/fs.h>           // Необходимо для работы с файлами устройств
#include <linux/cdev.h>         // Необходимо для работы с символьными устройствами
#include <linux/uaccess.h>      // Необходимо для copy_to_user и copy_from_user
#include <linux/random.h>

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define DEVICE_NAME "virttempsensor"  // Имя устройства
#define CLASS_NAME  "vts"             // Имя класса устройства

static int    majorNumber;             // Мажорный номер устройства
static struct class*  charClass  = NULL; 
static struct device* charDevice = NULL;
static struct cdev c_dev;              // Структура символьного устройства

static struct file_operations fops =
{
   .open = device_open,
   .read = device_read,
   .write = device_write,
   .release = device_release,
};

static int __init chardev_init(void) {
   printk(KERN_INFO "VTS: Initializing the VTS LKM\n");

   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber < 0) {
      printk(KERN_ALERT "VTS failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "VTS: registered correctly with major number %d\n", majorNumber);

   charClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(charClass)) {
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(charClass);
   }
   printk(KERN_INFO "VTS: device class registered correctly\n");

   charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(charDevice)) {
      class_destroy(charClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(charDevice);
   }
   printk(KERN_INFO "VTS: device class created correctly\n");

   cdev_init(&c_dev, &fops);
   if (cdev_add(&c_dev, MKDEV(majorNumber, 0), 1) == -1) {
      device_destroy(charClass, MKDEV(majorNumber, 0));
      class_destroy(charClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to add cdev\n");
      return -1;
   }

   return 0;
}

static void __exit chardev_exit(void) {
   cdev_del(&c_dev);
   device_destroy(charClass, MKDEV(majorNumber, 0));
   class_destroy(charClass);
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk(KERN_INFO "VTS: Goodbye from the VTS LKM!\n");
}

static int device_open(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "VTS: Device has been opened\n");
   return 0;
}

static int get_random_temperature(void) {
    unsigned int temp;
    get_random_bytes(&temp, sizeof(temp));
    return temp % 100;
}

static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
   int errors = 0;
   char temperature_value[5];
   int temp = get_random_temperature();  // Функция для получения случайной температуры

   snprintf(temperature_value, sizeof(temperature_value), "%d\n", temp);
   errors = copy_to_user(buffer, temperature_value, strlen(temperature_value));

   if (errors == 0) {
      printk(KERN_INFO "VTS: Sent %zu characters to the user\n", strlen(temperature_value));
      return strlen(temperature_value);
   } else {
      printk(KERN_INFO "VTS: Failed to send %d characters to the user\n", errors);
      return -EFAULT; // Ошибка - не удалось отправить данные пользователю
   }
}




static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
   printk(KERN_INFO "VTS: Sorry, write operations are not supported.\n");
   return -EFAULT; // Ошибка - операция записи не поддерживается
}

static int device_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "VTS: Device successfully closed\n");
   return 0;
}



module_init(chardev_init);
module_exit(chardev_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("roman");
MODULE_DESCRIPTION("Символьный драйвер для виртуального датчика температуры");

