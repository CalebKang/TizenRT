/*
 * stm32_exflash.c
 *
 *  Created on: Sep 30, 2019
 *      Author: calebkang
 */
#include <tinyara/config.h>

#include <debug.h>

#include <tinyara/arch.h>
#include <tinyara/board.h>
#include <tinyara/spi/ospi.h>

#include <arch/board/board.h>
#include <arch/board/stm32l4r9i_discovery_io.h>
#include <arch/board/stm32l4r9ai-disco.h>

#include "up_arch.h"

struct ospi_dev_s *g_ospi_ops;

#define EXFLASH_BUFF_LEN        (100)
static uint8_t exFlashRxData[EXFLASH_BUFF_LEN];
static uint8_t exFlashTxData[EXFLASH_BUFF_LEN]="**********1234567890**********1234567890**********1234567890**********1234567890**********1234567890";
static void initial_exflash_check(void)
{
  int i;

  stm32_exflash_read(exFlashRxData, 0x0, EXFLASH_BUFF_LEN);

  for(i=0; i<EXFLASH_BUFF_LEN; i++)
  {
    if(exFlashRxData[i] != exFlashTxData[i])
      break;
  }

  if(i != EXFLASH_BUFF_LEN)
  {
    stm32_exflash_erase_block(0);
    stm32_exflash_write(exFlashTxData, 0x0, EXFLASH_BUFF_LEN);
  }
}

/****************************************************************************
 * Name: stm32_exflash_initialize
 *
 * Description:
 *   Put the OSPI device into memory mapped mode
 *
 * Input Parameters:
 *   dev - OSPI device
 *   meminfo - parameters like for a memory transfer used for reading
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void stm32_exflash_initialize(void)
{
  g_ospi_ops = stm32l4_ospi_initialize(0);
}

#if 0
  uint8_t   flags;       /* See QSPMEM_* definitions */
  uint8_t   addrlen;     /* Address length in bytes */
  uint8_t   dummies;     /* Number of dummy read cycles (READ only) */
  uint16_t  buflen;      /* Data buffer length in bytes */
  uint16_t  cmd;         /* Memory access command */
  uint32_t  addr;        /* Memory Address */
  uint32_t  key;         /* Scrambler key */
  FAR void *buffer;      /* Data buffer */
#endif
void stm32_exflash_write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
  struct ospi_meminfo_s meminfo;

  meminfo.flags = OSPIMEM_WRITE;
  meminfo.addr = WriteAddr;
  meminfo.buffer = (uint8_t*)pData;
  meminfo.buflen = Size;

  int error = g_ospi_ops->ops->memory(g_ospi_ops, &meminfo);

  if(error != 0/*OSPI_NOR_OK*/)
  {
    printf("Flash write failed : %d!!\n", error);
  }
  else
  {
    printf("Flash write succeeded!!\n");
  }
}


/****************************************************************************
 * Name: stm32_exflash_read
 *
 * Description:
 *   Put the OSPI device into memory mapped mode
 *
 * Input Parameters:
 *   dev - OSPI device
 *   meminfo - parameters like for a memory transfer used for reading
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void stm32_exflash_read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
  struct ospi_meminfo_s meminfo;

  meminfo.flags = OSPIMEM_READ;
  meminfo.addr = ReadAddr;
  meminfo.buffer = (uint8_t*)pData;
  meminfo.buflen = Size;

  int error = g_ospi_ops->ops->memory(g_ospi_ops, &meminfo);

  if(error != 0/*OSPI_NOR_OK*/)
  {
    printf("Flash read failed : %d!!\n", error);
  }
  else
  {
    printf("Flash read succeeded!!\n");
  }
}


/****************************************************************************
 * Name: stm32_exflash_erase_block
 *
 * Description:
 *   Put the OSPI device into memory mapped mode
 *
 * Input Parameters:
 *   dev - OSPI device
 *   meminfo - parameters like for a memory transfer used for reading
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void stm32_exflash_erase_block(uint32_t block)
{
  struct ospi_meminfo_s meminfo;

  meminfo.flags = OSPIMEM_ERASEBLCK;
  meminfo.addr = block;

  int error = g_ospi_ops->ops->memory(g_ospi_ops, &meminfo);

  if(error != 0/*OSPI_NOR_OK*/)
  {
    printf("Flash erase block failed : %d!!\n", error);
  }
  else
  {
    printf("Flash erase block succeeded!!\n");
  }
}


/****************************************************************************
 * Name: stm32_exflash_erase_sector
 *
 * Description:
 *   Put the OSPI device into memory mapped mode
 *
 * Input Parameters:
 *   dev - OSPI device
 *   meminfo - parameters like for a memory transfer used for reading
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void stm32_exflash_erase_sector(uint32_t sector)
{
}

/****************************************************************************
 * Name: stm32l4_ospi_enter_memorymapped
 *
 * Description:
 *   Put the OSPI device into memory mapped mode
 *
 * Input Parameters:
 *   dev - OSPI device
 *   meminfo - parameters like for a memory transfer used for reading
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void stm32_exflash_enter_memorymapped(void)
{
  stm32l4_ospi_enter_memorymapped(g_ospi_ops, NULL, 0);
}


/****************************************************************************
 * Name: stm32l4_ospi_enter_memorymapped
 *
 * Description:
 *   Put the OSPI device into memory mapped mode
 *
 * Input Parameters:
 *   dev - OSPI device
 *   meminfo - parameters like for a memory transfer used for reading
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void stm32_exflash_exit_memorymapped(void)
{
  stm32l4_ospi_exit_memorymapped(g_ospi_ops);
}









