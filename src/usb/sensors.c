/*
 * sensors.c
 *
 *  Created on: Jul 9, 2018
 *      Author: technix
 */

#include "usb/features.h"
#include "usb/descriptor.h"
#include <stddef.h>

usbd_respond sensors_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback)
{
	uint8_t descriptor_type = (req->wValue & 0xff00) >> 8;
	__attribute__((unused))  uint8_t descriptor_idx = req->wValue & 0xff;

	if (req->bRequest == USB_STD_GET_DESCRIPTOR)
	{
		switch (descriptor_type)
		{
		case USB_DTYPE_HID_REPORT:
			dev->status.data_ptr = (void *)sensors_report_descriptor;
			dev->status.data_count = sizeof(sensors_report_descriptor);
			return usbd_ack;

		default:
			return usbd_fail;
		}
	}

	return usbd_fail;
}

void sensors_handle(usbd_device *dev, uint8_t event, uint8_t ep)
{
	switch (event)
	{
	case usbd_evt_eptx:
		usbd_ep_write(dev, ep, NULL, 0);
		break;

	case usbd_evt_eprx:
		usbd_ep_read(dev, ep, NULL, 0);
		break;

	default:
		break;
	}
}
