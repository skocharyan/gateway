

#include "FreeRTOS.h"
#include "eth.h"


BaseType_t prvSetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString)
{
    const char* pcParameter1;
    const char* pcParameter2;
    BaseType_t lParameter1StringLength, lParameter2StringLength;

    // Extract parameters
    pcParameter1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParameter1StringLength);
    if (pcParameter1 == NULL) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: missing first parameter\r\n");
        return pdFALSE;
    }

    pcParameter2 = FreeRTOS_CLIGetParameter(pcCommandString, 2, &lParameter2StringLength);
    if (pcParameter2 == NULL || lParameter2StringLength == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: missing second parameter\r\n");
        return pdFALSE;
    }

    // Ensure parameters are null-terminated
    char param1[20] = { 0 };
    char param2[32] = { 0 };

    strncpy(param1, pcParameter1, lParameter1StringLength);
    param1[lParameter1StringLength] = '\0';

    strncpy(param2, pcParameter2, lParameter2StringLength);
    param2[lParameter2StringLength] = '\0';

    // Match and apply configuration
    if (strcmp(param1, "port") == 0) {
        uint16_t portNumber = (uint16_t)strtoul(param2, NULL, 10);
        // dispenser_state.configuration.ethernetConfig.portNumber = portNumber;
        snprintf(pcWriteBuffer, xWriteBufferLen, "Port set to %u\r\n", portNumber);
    }
    else if (strcmp(param1, "ip") == 0) {

        uint32_t ipAddress = FreeRTOS_inet_addr(param2);

        if (ipAddress == 0) {
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid IP address format\r\n");
            return pdFALSE;
        }
        // memcpy(dispenser_state.configuration.ethernetConfig.ipAddress, &ipAddress, sizeof(ipAddress));

        snprintf(pcWriteBuffer, xWriteBufferLen, "IP set to %s\r\n", param2);
    }
    else if (strcmp(param1, "mask") == 0)
    {
        uint32_t mask = FreeRTOS_inet_addr(param2);
        if (mask == 0)
        {
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid mask format\r\n");
            return pdFALSE;
        }
        // memcpy(dispenser_state.configuration.ethernetConfig.subnetMask,
        //     &mask, sizeof(mask));
        snprintf(pcWriteBuffer, xWriteBufferLen, "Mask set to %s\r\n", param2);
    }
    else if (strcmp(param1, "gateway") == 0)
    {
        uint32_t gw = FreeRTOS_inet_addr(param2);
        if (gw == 0)
        {
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid gateway format\r\n");
            return pdFALSE;
        }
        // memcpy(dispenser_state.configuration.ethernetConfig.gatewayAddress,
        //     &gw, sizeof(gw));
        snprintf(pcWriteBuffer, xWriteBufferLen, "Gateway set to %s\r\n", param2);
    }
    else if (strcmp(param1, "dns") == 0)
    {
        uint32_t dns = FreeRTOS_inet_addr(param2);
        if (dns == 0)
        {
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid DNS format\r\n");
            return pdFALSE;
        }
        // memcpy(dispenser_state.configuration.ethernetConfig.dnsAddress,
        //     &dns, sizeof(dns));
        snprintf(pcWriteBuffer, xWriteBufferLen, "DNS set to %s\r\n", param2);
    }
    else if (strcmp(param1, "id") == 0) {
        if (lParameter2StringLength > 15) { // Limit to 15 characters
            snprintf(pcWriteBuffer, xWriteBufferLen, "Error: ID must be at most 15 characters\r\n");
            return pdFALSE;
        }
        // memset(dispenser_state.configuration.deviceId, 0, sizeof(dispenser_state.configuration.deviceId));
        // strncpy((char*)dispenser_state.configuration.deviceId, param2, lParameter2StringLength);
        // dispenser_state.configuration.deviceId[lParameter2StringLength] = '\0'; // Ensure null-termination
        // snprintf(pcWriteBuffer, xWriteBufferLen, "ID set to %s\r\n", dispenser_state.configuration.deviceId);
    }

    else {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: unknown parameter %s\r\n", param1);
        return pdFALSE;
    }

    // FRAM_write(FRAM_START_ADDRESS, sizeof(dispenser_state.configuration), (uint8_t*)&dispenser_state.configuration);

    return pdFALSE;
}



BaseType_t prvGetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString)
{
    const char* pcParameter;
    BaseType_t lParameterLength;

    // pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParameterLength);

    // // If no parameter or parameter == "all", dump everything
    // if (pcParameter == NULL
    //     || (lParameterLength == 3 && strncmp(pcParameter, "all", 3) == 0)) {
    //     const char* ethMode =
    //         (dispenser_state.configuration.ethernetConfig.dhcpMode == DHCP_STATIC) ? "STATIC" :
    //         (dispenser_state.configuration.ethernetConfig.dhcpMode == DHCP_DYNAMIC) ? "DYNAMIC" :
    //         "OFF";
    //     snprintf(pcWriteBuffer, xWriteBufferLen,
    //         "ID: %s\r\n" // Include ID
    //         "IP: %u.%u.%u.%u\r\n"
    //         "Port: %lu\r\n"
    //         "Mask: %u.%u.%u.%u\r\n"
    //         "Gateway: %u.%u.%u.%u\r\n"
    //         "DNS: %u.%u.%u.%u\r\n"
    //         "Ethernet: %s\r\n"
    //         ,
    //         dispenser_state.configuration.deviceId,
    //         dispenser_state.configuration.ethernetConfig.ipAddress[0],
    //         dispenser_state.configuration.ethernetConfig.ipAddress[1],
    //         dispenser_state.configuration.ethernetConfig.ipAddress[2],
    //         dispenser_state.configuration.ethernetConfig.ipAddress[3],
    //         dispenser_state.configuration.ethernetConfig.portNumber,
    //         dispenser_state.configuration.ethernetConfig.subnetMask[0],
    //         dispenser_state.configuration.ethernetConfig.subnetMask[1],
    //         dispenser_state.configuration.ethernetConfig.subnetMask[2],
    //         dispenser_state.configuration.ethernetConfig.subnetMask[3],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[0],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[1],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[2],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[3],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[0],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[1],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[2],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[3],
    //         ethMode
    //     );

    //     return pdFALSE;
    // }
    // else if (strncmp(pcParameter, "id", lParameterLength) == 0) {
    //     snprintf(pcWriteBuffer, xWriteBufferLen, "ID: %s\r\n", dispenser_state.configuration.deviceId);
    // }
    // else if (strncmp(pcParameter, "ip", lParameterLength) == 0)
    // {
    //     snprintf(pcWriteBuffer, xWriteBufferLen,
    //         "IP: %u.%u.%u.%u\r\n",
    //         dispenser_state.configuration.ethernetConfig.ipAddress[0],
    //         dispenser_state.configuration.ethernetConfig.ipAddress[1],
    //         dispenser_state.configuration.ethernetConfig.ipAddress[2],
    //         dispenser_state.configuration.ethernetConfig.ipAddress[3]);
    // }
    // else if (strncmp(pcParameter, "port", lParameterLength) == 0)
    // {
    //     snprintf(pcWriteBuffer, xWriteBufferLen, "Port: %lu\r\n", dispenser_state.configuration.ethernetConfig.portNumber);
    // }
    // // mask
    // else if (strncmp(pcParameter, "mask", lParameterLength) == 0)
    // {
    //     snprintf(pcWriteBuffer, xWriteBufferLen,
    //         "Mask: %u.%u.%u.%u\r\n",
    //         dispenser_state.configuration.ethernetConfig.subnetMask[0],
    //         dispenser_state.configuration.ethernetConfig.subnetMask[1],
    //         dispenser_state.configuration.ethernetConfig.subnetMask[2],
    //         dispenser_state.configuration.ethernetConfig.subnetMask[3]);
    // }

    // // gateway
    // else if (strncmp(pcParameter, "gateway", lParameterLength) == 0)
    // {
    //     snprintf(pcWriteBuffer, xWriteBufferLen,
    //         "Gateway: %u.%u.%u.%u\r\n",
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[0],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[1],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[2],
    //         dispenser_state.configuration.ethernetConfig.gatewayAddress[3]);
    // }

    // // dns
    // else if (strncmp(pcParameter, "dns", lParameterLength) == 0)
    // {
    //     snprintf(pcWriteBuffer, xWriteBufferLen,
    //         "DNS: %u.%u.%u.%u\r\n",
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[0],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[1],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[2],
    //         dispenser_state.configuration.ethernetConfig.dnsAddress[3]);
    // }
    // else if (strncmp(pcParameter, "eth", lParameterLength) == 0
    //     || strncmp(pcParameter, "ethernet", lParameterLength) == 0)
    // {
    //     const char* ethMode =
    //         (dispenser_state.configuration.ethernetConfig.dhcpMode == DHCP_STATIC) ? "STATIC" :
    //         (dispenser_state.configuration.ethernetConfig.dhcpMode == DHCP_DYNAMIC) ? "DYNAMIC" :
    //         "OFF";
    //     snprintf(pcWriteBuffer, xWriteBufferLen,
    //         "Ethernet: %s\r\n",
    //         ethMode);
    // }
    // else
    // {
    //     snprintf(pcWriteBuffer, xWriteBufferLen, "Error: unknown parameter %.*s\r\n", (int)lParameterLength, pcParameter);
    // }

    return pdFALSE;

}
