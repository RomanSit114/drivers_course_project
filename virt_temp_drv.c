/*#include <linux/module.h>       // Необходимо для всех модулей ядра
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
*/

// Включение необходимых заголовочных файлов для работы с модулями ядра, файловой системой, символьными устройствами и другими функциями ядра
#include <linux/module.h>       // Для всех модулей ядра
#include <linux/kernel.h>       // Для использования KERN_INFO
#include <linux/fs.h>           // Для работы с файлами устройств
#include <linux/cdev.h>         // Для работы с символьными устройствами
#include <linux/uaccess.h>      // Для copy_to_user и copy_from_user
#include <linux/random.h>       // Для работы со случайными числами

// Прототипы функций, используемых модулем
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

// Определение имени устройства и класса устройства
#define DEVICE_NAME "virttempsensor"
#define CLASS_NAME  "vts"

// Глобальные переменные для драйвера
static int    majorNumber;             // Мажорный номер устройства
static struct class*  charClass  = NULL; // Указатель на класс устройства
static struct device* charDevice = NULL; // Указатель на структуру устройства
static struct cdev c_dev;              // Структура символьного устройства

// Структура, определяющая операции с файлом устройства
static struct file_operations fops = {
   .open = device_open,
   .read = device_read,
   .write = device_write,
   .release = device_release,
};

// Функция инициализации модуля
static int __init chardev_init(void) {
   printk(KERN_INFO "VTS: Initializing the VTS LKM\n");

   // Регистрация устройства и получение мажорного номера
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber < 0) {
      printk(KERN_ALERT "VTS failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "VTS: registered correctly with major number %d\n", majorNumber);

   // Создание класса устройства
   charClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(charClass)) {
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(charClass);
   }
   printk(KERN_INFO "VTS: device class registered correctly\n");

   // Создание устройства
   charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(charDevice)) {
      class_destroy(charClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(charDevice);
   }
   printk(KERN_INFO "VTS: device class created correctly\n");

   // Инициализация c_dev с нашими fops
   cdev_init(&c_dev, &fops);
   // Добавление c_dev в ядро
   if (cdev_add(&c_dev, MKDEV(majorNumber, 0), 1) == -1) {
      device_destroy(charClass, MKDEV(majorNumber, 0));
      class_destroy(charClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to add cdev\n");
      return -1;
   }

   return 0;
}

// Функция выгрузки модуля
static void __exit chardev_exit(void) {
   cdev_del(&c_dev); // Удаление c_dev
   device_destroy(charClass, MKDEV(majorNumber, 0)); // Уничтожение устройства
   class_destroy(charClass); // Уничтожение класса устройства
   unregister_chrdev(majorNumber, DEVICE_NAME); // Отмена регистрации мажорного номера
   printk(KERN_INFO "VTS: Goodbye from the VTS LKM!\n");
}

// Функция, вызываемая при открытии файла устройства
static int device_open(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "VTS: Device has been opened\n");
   return 0;
}

// Функция для генерации "температуры"
static int get_random_temperature(void) {
    unsigned int temp;
    get_random_bytes(&temp, sizeof(temp)); // Получение случайных байтов
    return temp % 100; // Возвращение значения температуры
}

// Функция, вызываемая при чтении файла устройства
static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
   int errors = 0;
   char temperature_value[5]; // Буфер для значения температуры
   int temp = get_random_temperature(); // Получение "температуры"

   // Запись значения температуры в буфер
   snprintf(temperature_value, sizeof(temperature_value), "%d\n", temp);
   // Копирование данных из буфера ядра в буфер пользователя
   errors = copy_to_user(buffer, temperature_value, strlen(temperature_value));

   // Проверка на успешное копирование
   if (errors == 0) {
      printk(KERN_INFO "VTS: Sent %zu characters to the user\n", strlen(temperature_value));
      return strlen(temperature_value); // Возвращение количества переданных байт
   } else {
      printk(KERN_INFO "VTS: Failed to send %d characters to the user\n", errors);
      return -EFAULT; // Возвращение ошибки
   }
}

// Функция, вызываемая при записи в файл устройства
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
   printk(KERN_INFO "VTS: Sorry, write operations are not supported.\n");
   return -EFAULT; // Операция записи не поддерживается
}

// Функция, вызываемая при закрытии файла устройства
static int device_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "VTS: Device successfully closed\n");
   return 0;
}

// Макросы для инициализации и выгрузки модуля
module_init(chardev_init);
module_exit(chardev_exit);

// Метаинформация о модуле
MODULE_LICENSE("GPL"); // Лицензия модуля
MODULE_AUTHOR("roman"); // Автор модуля
MODULE_DESCRIPTION("Символьный драйвер для виртуального датчика температуры"); // Описание модуля

