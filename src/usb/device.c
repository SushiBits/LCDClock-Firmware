/*
 * device.c
 *
 *  Created on: Jul 9, 2018
 *      Author: technix
 */

#include "usb/features.h"
#include "usb/descriptor.h"
#include <stddef.h>

void USB_IRQHandler(void);
__attribute__((alias("USB_IRQHandler")))void USB_HP_CAN1_TX_IRQHandler(void);
__attribute__((alias("USB_IRQHandler")))void USB_LP_CAN1_RX0_IRQHandler(void);
__attribute__((alias("USB_IRQHandler")))void USBWakeUp_IRQHandler(void);

static usbd_device usb_device;
static uint32_t usb_buffer[USB_EP0_SIZE];

static usbd_respond usb_get_descriptor(usbd_ctlreq *req, void **address, uint16_t *length);
static usbd_respond usb_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback);
static usbd_respond usb_set_config(usbd_device *dev, uint8_t cfg);

__attribute__((constructor)) void usb_init(void)
{
	NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_EnableIRQ(USBWakeUp_IRQn);

	usbd_init(&usb_device, &usbd_devfs, USB_EP0_SIZE, usb_buffer, sizeof(usb_buffer));
	usbd_reg_descr(&usb_device, usb_get_descriptor);
	usbd_reg_control(&usb_device, usb_control);
	usbd_reg_config(&usb_device, usb_set_config);
	__DSB();

	usbd_enable(&usb_device, true);
	usbd_connect(&usb_device, true);
	__DSB();
}

void USB_IRQHandler(void)
{
	usbd_poll(&usb_device);
}

static usbd_respond usb_get_descriptor(usbd_ctlreq *req, void **address, uint16_t *length)
{
	uint8_t descriptor_type = (req->wValue & 0xff00) >> 8;
	uint8_t descriptor_idx = req->wValue & 0xff;

	const void *buf = NULL;
	uint16_t len = 0;

	switch (descriptor_type)
	{
	case USB_DTYPE_DEVICE:
		buf = &device_descriptor;
		break;

	case USB_DTYPE_CONFIGURATION:
		buf = &config_descriptor;
		len = sizeof(config_descriptor);
		break;

	case USB_DTYPE_QUALIFIER:
		buf = &qualifier_descriptor;
		break;

	case USB_DTYPE_STRING:
		if (descriptor_idx < sizeof(desc_string_table) / sizeof(struct usb_string_descriptor))
			buf = desc_string_table[descriptor_idx];
		break;

	default:
		break;
	}

	if (!buf)
		return usbd_fail;
	len = len ?: ((struct usb_device_descriptor *)buf)->bLength;

	if (address)
		*address = (void *)buf;
	if (length)
		*length = len;

	return usbd_ack;
}

static usbd_respond usb_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback)
{
	if ((req->bmRequestType & USB_REQ_RECIPIENT) == USB_REQ_INTERFACE)
	{
		switch (req->wIndex)
		{
		case 0:
			return sensors_control(dev, req, callback);

		case 1:
			return config_control(dev, req, callback);

		default:
			return usbd_fail;
		}
	}

	if ((req->bmRequestType & USB_REQ_RECIPIENT) == USB_REQ_ENDPOINT)
	{
		switch (req->wIndex)
		{
		case SENSORS_EP_IN:
		case SENSORS_EP_OUT:
			return sensors_control(dev, req, callback);

		case CONFIG_EP_IN:
		case CONFIG_EP_OUT:
			return config_control(dev, req, callback);

		default:
			return usbd_fail;
		}
	}

	return usbd_fail;
}

static usbd_respond usb_set_config(usbd_device *dev, uint8_t cfg)
{
	switch (cfg)
	{
	case 0:
		// Deconfigure everything.
		usbd_ep_deconfig(&usb_device, SENSORS_EP_IN);
		usbd_ep_deconfig(&usb_device, SENSORS_EP_OUT);
		usbd_reg_endpoint(&usb_device, SENSORS_EP_IN, NULL);
		usbd_reg_endpoint(&usb_device, SENSORS_EP_OUT, NULL);

		usbd_ep_deconfig(&usb_device, CONFIG_EP_IN);
		usbd_ep_deconfig(&usb_device, CONFIG_EP_OUT);
		usbd_reg_endpoint(&usb_device, CONFIG_EP_IN, NULL);
		usbd_reg_endpoint(&usb_device, CONFIG_EP_OUT, NULL);
		return usbd_ack;

	case 1:
		usbd_ep_config(dev, SENSORS_EP_IN, USB_EPTYPE_INTERRUPT, SENSORS_EP_SIZE);
		usbd_ep_config(dev, SENSORS_EP_OUT, USB_EPTYPE_INTERRUPT, SENSORS_EP_SIZE);
		usbd_reg_endpoint(dev, SENSORS_EP_IN, sensors_handle);
		usbd_reg_endpoint(dev, SENSORS_EP_OUT, sensors_handle);
		usbd_ep_write(dev, SENSORS_EP_OUT, NULL, 0);

		usbd_ep_config(dev, CONFIG_EP_IN, USB_EPTYPE_INTERRUPT, SENSORS_EP_SIZE);
		usbd_ep_config(dev, CONFIG_EP_OUT, USB_EPTYPE_INTERRUPT, SENSORS_EP_SIZE);
		usbd_reg_endpoint(dev, CONFIG_EP_IN, config_handle);
		usbd_reg_endpoint(dev, CONFIG_EP_OUT, config_handle);
		usbd_ep_write(dev, CONFIG_EP_OUT, NULL, 0);
        return usbd_ack;

	default:
		break;
	}
	return usbd_fail;
}
