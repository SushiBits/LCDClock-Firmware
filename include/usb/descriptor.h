/*
 * descriptor.h
 *
 *  Created on: Jul 9, 2018
 *      Author: technix
 */

#ifndef INCLUDE_USB_DESCRIPTOR_H_
#define INCLUDE_USB_DESCRIPTOR_H_

#include <stm32f1xx.h>
#include <usb.h>
#include <usbd_core.h>
#include <usb_std.h>
#include <usb_hid.h>

#define USB_EP0_SIZE	64

#define SENSORS_EP_IN	(USB_EPDIR_IN  | 0x01)
#define SENSORS_EP_OUT	(USB_EPDIR_OUT | 0x01)
#define SENSORS_EP_SIZE	10

#define CONFIG_EP_IN	(USB_EPDIR_IN  | 0x02)
#define CONFIG_EP_OUT	(USB_EPDIR_OUT | 0x02)
#define CONFIG_EP_SIZE	64

__attribute__((packed)) struct config_descriptor
{
	struct usb_config_descriptor	config;

	struct usb_interface_descriptor	sensors_interface;
	struct usb_hid_descriptor		sensors_descriptor;
	struct usb_endpoint_descriptor	sensors_endpoints[2];

	struct usb_interface_descriptor	config_interface;
	struct usb_hid_descriptor		config_descriptor;
	struct usb_endpoint_descriptor	config_endpoints[2];
};

static const struct usb_device_descriptor device_descriptor =
{
	    .bLength            = sizeof(struct usb_device_descriptor),
	    .bDescriptorType    = USB_DTYPE_DEVICE,
	    .bcdUSB             = VERSION_BCD(2,0,0),
	    .bDeviceClass       = USB_CLASS_PER_INTERFACE,
	    .bDeviceSubClass    = USB_SUBCLASS_NONE,
	    .bDeviceProtocol    = USB_PROTO_NONE,
	    .bMaxPacketSize0    = USB_EP0_SIZE,
	    .idVendor           = 0x0002,
	    .idProduct          = 0xc002,
	    .bcdDevice          = VERSION_BCD(0,2,0),
	    .iManufacturer      = 1,
	    .iProduct           = 2,
	    .iSerialNumber      = INTSERIALNO_DESCRIPTOR,
	    .bNumConfigurations = 1,
};

static const struct usb_qualifier_descriptor qualifier_descriptor =
{
	    .bLength            = sizeof(struct usb_qualifier_descriptor),
	    .bDescriptorType    = USB_DTYPE_QUALIFIER,
	    .bcdUSB             = VERSION_BCD(2, 0, 0),
	    .bDeviceClass       = USB_CLASS_PER_INTERFACE,
	    .bDeviceSubClass    = USB_SUBCLASS_NONE,
	    .bDeviceProtocol    = USB_PROTO_NONE,
	    .bMaxPacketSize0    = USB_EP0_SIZE,
	    .bNumConfigurations = 1,
	    .bReserved          = 0
};

static const uint8_t sensors_report_descriptor[] =
{
		HID_RI_USAGE_PAGE(8, 0x20),					// Usage Page: Sensor
		HID_RI_USAGE(8, 0x01),						// Usage: Sensor
		HID_RI_COLLECTION(8, 0x00),					// Collection: Physical
			HID_RI_USAGE(8, 0x33),					//   Usage: Temperature
			HID_RI_UNIT(32, 0x01000100),			//   Unit: Kelvin
			HID_RI_UNIT_EXPONENT(8, 0x0e),			//   Unit Exponent: 10^-2
			HID_RI_LOGICAL_MINIMUM(8, 0x00),		//   Logical Minimum: 0.00K
			HID_RI_LOGICAL_MAXIMUM(16, 0xffff),		//   Logical Maximum: 655.35K
			HID_RI_REPORT_SIZE(8, 16),				//   Size: 16 bits
			HID_RI_REPORT_COUNT(8, 1),				//   Count: 1
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

			HID_RI_USAGE(8, 0x32),					//   Usage: Humidity
			HID_RI_UNIT(8, 0),						//   Unit: None (1)
			HID_RI_LOGICAL_MINIMUM(8, 0),			//   Logical Minimum: 0.00%
			HID_RI_LOGICAL_MAXIMUM(16, 10000),		//   Logical Maximum: 100.00%
			HID_RI_REPORT_SIZE(8, 16),				//   Size: 16 bits
			HID_RI_REPORT_COUNT(8, 1),				//   Count: 1
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

			HID_RI_USAGE(8, 0x26),					//   Usage: Voltage
			HID_RI_UNIT(32, 0x00f0d121),			//   Unit: Volt
			HID_RI_UNIT_EXPONENT(8, 0x0c),			//   Unit Exponent: 10^-4
			HID_RI_LOGICAL_MINIMUM(8, 0x00),		//   Logical Minimum: 0.0V
			HID_RI_LOGICAL_MAXIMUM(16, 0xffff),		//   Logical Maximum: 6.5535V
			HID_RI_REPORT_SIZE(8, 16),				//   Size: 16 bits
			HID_RI_REPORT_COUNT(8, 1),				//   Count: 1
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

			HID_RI_USAGE(8, 0xa2),					//   Usage: Real-time clock
			HID_RI_UNIT(16, 0x1001),				//   Unit: Second
			HID_RI_UNIT_EXPONENT(8, 0x00),			//   Unit Exponent: 1
			HID_RI_LOGICAL_MINIMUM(8, 0x00),		//   Logical Minimum: 0x00
			HID_RI_LOGICAL_MAXIMUM(32, 0xffffffff),	//   Logical Maximum: 0xffffffff
			HID_RI_REPORT_SIZE(8, 32),				//   Size: 32 bits
			HID_RI_REPORT_COUNT(8, 1),				//   Count: 1
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

			HID_RI_USAGE(8, 0xa2),					//   Usage: Real-time clock
			HID_RI_UNIT(16, 0x1001),				//   Unit: Second
			HID_RI_UNIT_EXPONENT(8, 0x00),			//   Unit Exponent: 1
			HID_RI_LOGICAL_MINIMUM(8, 0x00),		//   Logical Minimum: 0x00
			HID_RI_LOGICAL_MAXIMUM(32, 0xffffffff),	//   Logical Maximum: 0xffffffff
			HID_RI_REPORT_SIZE(8, 32),				//   Size: 32 bits
			HID_RI_REPORT_COUNT(8, 1),				//   Count: 1
			HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		HID_RI_END_COLLECTION(0),					// End Collection
};

static const uint8_t config_report_descriptor[] =
{
		HID_RI_USAGE_PAGE(16, 0xff00),				// Usage Page: Application Specific
		HID_RI_USAGE(8, 0x01),						// Usage: Application
		HID_RI_COLLECTION(8, 0x01),					// Collection: Application
			HID_RI_USAGE(8, 0x01),					//   Usage: Application
			HID_RI_LOGICAL_MINIMUM(8, 0x00),		//   Logical Minimum: 0x00
			HID_RI_LOGICAL_MAXIMUM(8, 0xff),		//   Logical Maximum: 0xff
			HID_RI_REPORT_SIZE(8, 8),				//   Size: 32 bits
			HID_RI_REPORT_COUNT(8, CONFIG_EP_SIZE),	//   Count: CONFIG_EP_SIZE
			HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

			HID_RI_USAGE(8, 0x01),					//   Usage: Application
			HID_RI_LOGICAL_MINIMUM(8, 0x00),		//   Logical Minimum: 0x00
			HID_RI_LOGICAL_MAXIMUM(8, 0xff),		//   Logical Maximum: 0xff
			HID_RI_REPORT_SIZE(8, 8),				//   Size: 32 bits
			HID_RI_REPORT_COUNT(8, CONFIG_EP_SIZE),	//   Count: CONFIG_EP_SIZE
			HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
		HID_RI_END_COLLECTION(0),					// End Collection
};

static const struct config_descriptor config_descriptor =
{
		.config =
		{
		        .bLength                = sizeof(struct usb_config_descriptor),
		        .bDescriptorType        = USB_DTYPE_CONFIGURATION,
		        .wTotalLength           = sizeof(struct config_descriptor),
		        .bNumInterfaces         = 2,
		        .bConfigurationValue    = 1,
		        .iConfiguration         = NO_DESCRIPTOR,
		        .bmAttributes           = USB_CFG_ATTR_RESERVED,
		        .bMaxPower              = USB_CFG_POWER_MA(500)
		},

		.sensors_interface =
		{
		        .bLength                = sizeof(struct usb_interface_descriptor),
		        .bDescriptorType        = USB_DTYPE_INTERFACE,
		        .bInterfaceNumber       = 0,
		        .bAlternateSetting      = 0,
		        .bNumEndpoints          = 2,
		        .bInterfaceClass        = USB_CLASS_HID,
		        .bInterfaceSubClass     = USB_SUBCLASS_NONE,
		        .bInterfaceProtocol     = USB_PROTO_NONE,
		        .iInterface             = NO_DESCRIPTOR,
		},
		.sensors_descriptor =
		{
				.bLength                = sizeof(struct usb_hid_descriptor),
				.bDescriptorType        = USB_DTYPE_HID,
				.bcdHID                 = VERSION_BCD(1, 1, 1),
				.bCountryCode           = USB_HID_COUNTRY_US,
				.bNumDescriptors        = 1,
				.bDescriptorType0       = USB_DTYPE_HID_REPORT,
				.wDescriptorLength0     = sizeof(sensors_report_descriptor)
		},
		.sensors_endpoints =
		{
				{
				        .bLength                = sizeof(struct usb_endpoint_descriptor),
				        .bDescriptorType        = USB_DTYPE_ENDPOINT,
				        .bEndpointAddress       = SENSORS_EP_IN,
				        .bmAttributes           = USB_EPTYPE_INTERRUPT,
				        .wMaxPacketSize         = SENSORS_EP_SIZE,
				        .bInterval              = 100,
				},
				{
				        .bLength                = sizeof(struct usb_endpoint_descriptor),
				        .bDescriptorType        = USB_DTYPE_ENDPOINT,
				        .bEndpointAddress       = SENSORS_EP_OUT,
				        .bmAttributes           = USB_EPTYPE_INTERRUPT,
				        .wMaxPacketSize         = SENSORS_EP_SIZE,
				        .bInterval              = 100,
				}
		},

		.config_interface =
		{
		        .bLength                = sizeof(struct usb_interface_descriptor),
		        .bDescriptorType        = USB_DTYPE_INTERFACE,
		        .bInterfaceNumber       = 1,
		        .bAlternateSetting      = 0,
		        .bNumEndpoints          = 2,
		        .bInterfaceClass        = USB_CLASS_HID,
		        .bInterfaceSubClass     = USB_SUBCLASS_NONE,
		        .bInterfaceProtocol     = USB_PROTO_NONE,
		        .iInterface             = NO_DESCRIPTOR,
		},
		.config_descriptor =
		{
				.bLength                = sizeof(struct usb_hid_descriptor),
				.bDescriptorType        = USB_DTYPE_HID,
				.bcdHID                 = VERSION_BCD(1, 1, 1),
				.bCountryCode           = USB_HID_COUNTRY_US,
				.bNumDescriptors        = 1,
				.bDescriptorType0       = USB_DTYPE_HID_REPORT,
				.wDescriptorLength0     = sizeof(config_report_descriptor)
		},
		.config_endpoints =
		{
				{
				        .bLength                = sizeof(struct usb_endpoint_descriptor),
				        .bDescriptorType        = USB_DTYPE_ENDPOINT,
				        .bEndpointAddress       = CONFIG_EP_IN,
				        .bmAttributes           = USB_EPTYPE_INTERRUPT,
				        .wMaxPacketSize         = CONFIG_EP_SIZE,
				        .bInterval              = 1,
				},
				{
				        .bLength                = sizeof(struct usb_endpoint_descriptor),
				        .bDescriptorType        = USB_DTYPE_ENDPOINT,
				        .bEndpointAddress       = CONFIG_EP_OUT,
				        .bmAttributes           = USB_EPTYPE_INTERRUPT,
				        .wMaxPacketSize         = CONFIG_EP_SIZE,
				        .bInterval              = 1,
				}
		}
};

static const struct usb_string_descriptor lang_desc     = USB_ARRAY_DESC(USB_LANGID_ENG_US);
static const struct usb_string_descriptor manuf_desc_en = USB_STRING_DESC("SushiBits by Max Chan <xcvista@me.com>");
static const struct usb_string_descriptor prod_desc_en  = USB_STRING_DESC("Climate Clock v2");
static const struct usb_string_descriptor *const desc_string_table[] =
{
		&lang_desc,
		&manuf_desc_en,
		&prod_desc_en,
};

#endif /* INCLUDE_USB_DESCRIPTOR_H_ */
