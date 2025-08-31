

#include "FreeRTOS.h"
#include "eth.h"
#include "flash.h"
#include "AppConfig.h"

extern EthernetConfig ethConfig;


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

    if (strcmp(param1, "port") == 0) {
        uint16_t portNumber = (uint16_t)strtoul(param2, NULL, 10);
        ethConfig.portNumber = portNumber;
        snprintf(pcWriteBuffer, xWriteBufferLen, "Port set to %u\r\n", portNumber);
    }
    else if (strcmp(param1, "ip") == 0) {

        uint32_t ipAddress = FreeRTOS_inet_addr(param2);

        if (ipAddress == 0) {
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid IP address format\r\n");
            return pdFALSE;
        }
        memcpy(ethConfig.ipAddress, &ipAddress, sizeof(ipAddress));
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
        memcpy(ethConfig.subnetMask, &mask, sizeof(mask));
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
        memcpy(ethConfig.gatewayAddress, &gw, sizeof(gw));
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
        memcpy(ethConfig.dnsAddress, &dns, sizeof(dns));
        snprintf(pcWriteBuffer, xWriteBufferLen, "DNS set to %s\r\n", param2);
    }
    else {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: unknown parameter %s\r\n", param1);
        return pdFALSE;
    }

    if (Flash_Write_Data(FLASH_CONFIG_ADDRESS, (uint32_t*)&ethConfig, sizeof(EthernetConfig) / 4) != 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: failed to write to flash\r\n");
    }
    return pdFALSE;
}



BaseType_t prvGetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString)
{
    const char* pcParameter;
    BaseType_t lParameterLength;

    pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParameterLength);

    const char* ethMode =
        (ethConfig.dhcpMode == DHCP_STATIC) ? "STATIC" :
        (ethConfig.dhcpMode == DHCP_DYNAMIC) ? "DYNAMIC" :
        "OFF";

    if (pcParameter == NULL || lParameterLength == 0 ||
        (lParameterLength == 3 && strncmp(pcParameter, "all", 3) == 0)) {
        snprintf(pcWriteBuffer, xWriteBufferLen,
            "IP: %u.%u.%u.%u\r\n"
            "Port: %lu\r\n"
            "Mask: %u.%u.%u.%u\r\n"
            "Gateway: %u.%u.%u.%u\r\n"
            "DNS: %u.%u.%u.%u\r\n"
            "DHCP: %s\r\n",
            ethConfig.ipAddress[0], ethConfig.ipAddress[1], ethConfig.ipAddress[2], ethConfig.ipAddress[3],
            (unsigned long)ethConfig.portNumber,
            ethConfig.subnetMask[0], ethConfig.subnetMask[1], ethConfig.subnetMask[2], ethConfig.subnetMask[3],
            ethConfig.gatewayAddress[0], ethConfig.gatewayAddress[1], ethConfig.gatewayAddress[2], ethConfig.gatewayAddress[3],
            ethConfig.dnsAddress[0], ethConfig.dnsAddress[1], ethConfig.dnsAddress[2], ethConfig.dnsAddress[3],
            ethMode);
        return pdFALSE;
    }

    char param[16] = { 0 };
    strncpy(param, pcParameter, (size_t)lParameterLength);
    param[lParameterLength < (BaseType_t)(sizeof(param) - 1) ? lParameterLength : (sizeof(param) - 1)] = '\0';

    if (strncmp(param, "ip", 2) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "IP: %u.%u.%u.%u\r\n",
            ethConfig.ipAddress[0], ethConfig.ipAddress[1], ethConfig.ipAddress[2], ethConfig.ipAddress[3]);
    }
    else if (strncmp(param, "port", 4) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Port: %lu\r\n", (unsigned long)ethConfig.portNumber);
    }
    else if (strncmp(param, "mask", 4) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Mask: %u.%u.%u.%u\r\n",
            ethConfig.subnetMask[0], ethConfig.subnetMask[1], ethConfig.subnetMask[2], ethConfig.subnetMask[3]);
    }
    else if (strncmp(param, "gateway", 7) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Gateway: %u.%u.%u.%u\r\n",
            ethConfig.gatewayAddress[0], ethConfig.gatewayAddress[1], ethConfig.gatewayAddress[2], ethConfig.gatewayAddress[3]);
    }
    else if (strncmp(param, "dns", 3) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "DNS: %u.%u.%u.%u\r\n",
            ethConfig.dnsAddress[0], ethConfig.dnsAddress[1], ethConfig.dnsAddress[2], ethConfig.dnsAddress[3]);
    }
    else if (strncmp(param, "dhcp", 4) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "DHCP: %s\r\n", ethMode);
    }
    else {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: unknown parameter %s\r\n", param);
    }
    return pdFALSE;
}


BaseType_t prvResetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString)
{
    const char* pcParameter;
    BaseType_t lParameterLength;

    pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &lParameterLength);

    if (pcParameter == NULL || lParameterLength == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Usage: reset <reboot|now>\r\n");
        return pdFALSE;
    }

    char param[16] = { 0 };
    strncpy(param, pcParameter, (size_t)lParameterLength);
    param[lParameterLength < (BaseType_t)(sizeof(param) - 1) ? lParameterLength : (sizeof(param) - 1)] = '\0';

    if (strncmp(param, "reboot", 6) == 0 || strncmp(param, "now", 3) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Rebooting...\r\n");
        HAL_Delay(50);
        HAL_NVIC_SystemReset();
        return pdFALSE;
    }

    snprintf(pcWriteBuffer, xWriteBufferLen, "Error: unknown parameter %s\r\n", param);
    return pdFALSE;
}