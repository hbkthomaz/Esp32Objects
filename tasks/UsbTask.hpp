#ifndef USB_TASK_HPP
#define USB_TASK_HPP

/**
 * @brief Creates Usb Task
 */
void UsbTaskCreate(void);
/**
 * @brief Task that reads data from usb and processes commands.
 */
void UsbTask(void *param);

#endif // USB_TASK_HPP
