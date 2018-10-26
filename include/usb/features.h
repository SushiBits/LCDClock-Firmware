/*
 * features.h
 *
 *  Created on: Jul 9, 2018
 *      Author: technix
 */

#ifndef INCLUDE_USB_FEATURES_H_
#define INCLUDE_USB_FEATURES_H_

#include <stm32f1xx.h>
#include <usb.h>
#include <usbd_core.h>
#include <usb_std.h>
#include <usb_hid.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

usbd_respond sensors_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback);
void sensors_handle(usbd_device *dev, uint8_t event, uint8_t ep);

usbd_respond config_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback);
void config_handle(usbd_device *dev, uint8_t event, uint8_t ep);

__END_DECLS

#endif /* INCLUDE_USB_FEATURES_H_ */
