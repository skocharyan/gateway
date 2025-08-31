#ifndef FLASH_H
#define FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

    uint32_t Flash_Write_Data(uint32_t StartSectorAddress, uint32_t* Data,
        uint16_t numberofwords);

    void Flash_Read_Data(uint32_t StartSectorAddress, uint32_t* RxBuf,
        uint16_t numberofwords);

#ifdef __cplusplus
}
#endif

#endif