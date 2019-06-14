#ifndef __Sa1110UsbStd_h__
#define __Sa1110UsbStd_h__

// Standard request codes
#define _GetStatus						0x00
#define _ClearFeature					0x01
#define _SetFeature						0x03
#define _SetAddress						0x05
#define _GetDescriptor					0x06
#define _SetDescriptor					0x07
#define _GetConfiguration				0x08
#define _SetConfiguration				0x09
#define _GetInterface					0x0A
#define _SetInterface					0x0B

// Recipients                   
#define _RecipientDevice				0x00
#define _RecipientInterface				0x01
#define _RecipientEndpoint				0x02
#define _RecipientOther					0x03

// Standard descriptors
#define _DeviceDescriptor				0x01
#define _ConfigurationDescriptor		0x02
#define _StringDescriptor				0x03
#define _InterfaceDescriptor			0x04
#define _EndpointDescriptor				0x05

// Vendor requests
#define _ResyncDevice					0x01
#define _WriteDevice					0x02
#define _ErrorSignature					0xE0

// Descriptor sizes
#define _DeviceDescriptorSize			18
#define _ConfigurationDescriptorSize	9
#define _InterfaceDescriptorSize		9
#define _EndpointDescriptorSize			7

// Feature selectors
#define _UnusedParameter(a)  			(a = a)

#define _MaxPacketSizeEndpoint0			8

#define _MaxPacketSizeEndpointOut		8
#define _MaxPacketSizeEndpointOut_1		7

// receive DMA buffer size
#define _MaxReceiveSize					0x500

#define _MaxPacketSizeEndpointIn		16 // 256
#define _MaxPacketSizeEndpointIn_1		15 // 255

#define _MaxBurstCount					1
#define _MaxBurstCount_1				0

//------------------------------------------------------------------------------------------------
// * USB working structures
//------------------------------------------------------------------------------------------------

typedef struct
{
	UInt8 requestType;
	UInt8 request;
	UInt16 value;
	UInt16 index;
	UInt16 length;
  
} DeviceRequest;

#endif // __Sa1110UsbStd_h__
