#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_Routing.h"
#include "eth.h"


EthernetConfig ethConfig;

uint8_t ucMACAddress[6];

NetworkInterface_t xInterfaces[1];
NetworkEndPoint_t xEndPoints[1];


// Private function prototypes
NetworkInterface_t* pxSTM32Fxx_FillInterfaceDescriptor(BaseType_t xEMACIndex,
    NetworkInterface_t* pxInterface);
void generate_mac_address(uint8_t* device_id, uint8_t* mac_buffer);
// EOF prototypes


void ethInit(void) {

    // generate_mac_address(dispenserID, ucMACAddress);

    pxSTM32Fxx_FillInterfaceDescriptor(0, &(xInterfaces[0]));

    if (memcmp(ethConfig.ipAddress, (uint8_t[]) { 0, 0, 0, 0 }, 4) == 0 ||
        memcmp(ethConfig.subnetMask, (uint8_t[]) { 0, 0, 0, 0 }, 4) == 0 ||
        memcmp(ethConfig.gatewayAddress, (uint8_t[]) { 0, 0, 0, 0 }, 4) == 0 ||
        ethConfig.portNumber == 0) {
        return;
    }

    FreeRTOS_FillEndPoint(
        &(xInterfaces[0]),
        &(xEndPoints[0]),
        ethConfig.ipAddress,
        ethConfig.subnetMask,
        ethConfig.gatewayAddress,
        ethConfig.dnsAddress,
        ucMACAddress
    );

#if ( ipconfigUSE_DHCP != 0 )
    {
        /* End-point 0 wants to use DHCPv4. */
        xEndPoints[0].bits.bWantDHCP = pdTRUE;
    }
#endif /* ( ipconfigUSE_DHCP != 0 ) */

    FreeRTOS_IPInit_Multi();

    //    ethTaskCreate();
}