#include "string.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"
#include "stm32f4xx_hal.h"


uint8_t ip_address[6] = { 192,168,1,133 };
uint16_t port = 1500;

void generate_mac_address(uint8_t* device_id, uint8_t* mac_buffer)
{
    // Set the first octet to 0x02 to indicate a locally administered MAC address
    mac_buffer[0] = 0x02;

    // Generate the remaining 5 octets by XOR-ing groups of the device_id bytes.
    // This loop ensures each of the five octets gets contributions from the 16 bytes.
    for (int i = 0; i < 5; i++) {
        uint8_t xor_val = 0;
        for (int j = i; j < 16; j += 5) {
            xor_val ^= device_id[j];
        }
        mac_buffer[i + 1] = xor_val;
    }
}


/* Send txLen bytes from pcTxBuffer, receive up to rxBufLen into pcRxBuffer.
   recvTimeoutMs is the receive timeout in milliseconds (socket option).
   Returns number of bytes received (>=0) or negative on error. */
int32_t vTCPSendAndReceive(const char* pcTxBuffer,
    size_t txLen,
    char* pcRxBuffer,
    size_t rxBufLen,
    TickType_t recvTimeoutMs)
{
    Socket_t xSocket = FREERTOS_INVALID_SOCKET;
    struct freertos_sockaddr xRemoteAddress;
    size_t alreadyTransmitted = 0;
    int32_t result = -1;

    if (pcTxBuffer == NULL || txLen == 0 || pcRxBuffer == NULL || rxBufLen == 0) {
        return -1;
    }

    memset(&xRemoteAddress, 0, sizeof(xRemoteAddress));
    xRemoteAddress.sin_port = FreeRTOS_htons(port);
    xRemoteAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr_quick(ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    xRemoteAddress.sin_family = FREERTOS_AF_INET4;

    xSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);
    if (xSocket == FREERTOS_INVALID_SOCKET) {
        return -1;
    }

    /* Set timeouts */
    {
        TickType_t xRecvTimeout = pdMS_TO_TICKS(recvTimeoutMs);
        TickType_t xSendTimeout = pdMS_TO_TICKS(5000);
        FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_RCVTIMEO, &xRecvTimeout, sizeof(xRecvTimeout));
        FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeout, sizeof(xSendTimeout));
    }

    if (FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress)) != 0) {
        FreeRTOS_closesocket(xSocket);
        return -1;
    }

    /* Send loop (handle partial sends). */
    while (alreadyTransmitted < txLen) {
        BaseType_t xBytesSent = FreeRTOS_send(xSocket,
            &pcTxBuffer[alreadyTransmitted],
            (size_t)(txLen - alreadyTransmitted),
            0);
        if (xBytesSent > 0) {
            alreadyTransmitted += (size_t)xBytesSent;
        }
        else {
            /* send error or timeout */
            FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
            FreeRTOS_closesocket(xSocket);
            return -1;
        }
    }

    /* Optional: tell peer no more data will be sent (depends on protocol). */
    (void)FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_WR);

    /* Receive loop */
    size_t alreadyReceived = 0;

    for (;;) {
        if (alreadyReceived >= rxBufLen) {
            break;
        }
        BaseType_t xBytesReceived = FreeRTOS_recv(xSocket,
            &pcRxBuffer[alreadyReceived],
            (size_t)(rxBufLen - alreadyReceived),
            0);
        if (xBytesReceived > 0) {
            alreadyReceived += (size_t)xBytesReceived;
            /* keep NUL termination when space permits */
            if (alreadyReceived < rxBufLen) {
                pcRxBuffer[alreadyReceived] = '\0';
            }
            /* continue to try to read more until timeout or remote closes */
            continue;
        }
        else if (xBytesReceived == 0) {
            /* No data (timeout or orderly close) */
            break;
        }
        else {
            /* Error */
            alreadyReceived = 0;
            break;
        }
    }

    (void)FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(xSocket);

    result = (int32_t)alreadyReceived;
    return result;
}

BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber) {

    *pulNumber = HAL_GetTick(); // Use the system tick as a random number source
    return pdTRUE;
}

uint32_t ulApplicationGetNextSequenceNumber(
    uint32_t ulSourceAddress,
    uint16_t usSourcePort,
    uint32_t ulDestinationAddress,
    uint16_t usDestinationPort)
{
    uint32_t ticks = xTaskGetTickCount();

    // Simple pseudo-random generation
    uint32_t value = ulSourceAddress ^ ulDestinationAddress;
    value ^= ((uint32_t)usSourcePort << 16) | usDestinationPort;
    value ^= ticks * 1103515245UL + 12345UL; // LCG

    return value;
}