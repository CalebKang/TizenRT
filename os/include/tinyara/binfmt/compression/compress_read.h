/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

#ifndef __INCLUDE_COMPRESS_READ_H
#define __INCLUDE_COMPRESS_READ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <tinyara/binfmt/compression/compression.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CUTOFF_RATIO_CACHE_BLOCKS 0.1f	/* Max. Ratio of blocks to be used for caching
					 * to total number of compressed blocks in binary */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Struct for output buffers to cache uncompressed blocks */
struct block_cache_s {
	unsigned char *out_buffer;			/* Buffer that is going to hold uncompressed data */
	int block_number;			/* Block number in compressed file for the cached block */
	unsigned int no_requests_for_block;	/* Number of requests for current block that is cached */
	unsigned int index_block_cache;		/* Index of block cache in the array */
	struct block_cache_s *next;		/* Pointer to next element in doubly linked list */
	struct block_cache_s *prev;		/* Pointer to previous element in doubly linked list */
};
typedef struct block_cache_s block_cache_t;

/****************************************************************************
 * Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: compress_uninit
 *
 * Description:
 *   Release buffers initialized during compress_init
 *
 * Returned Value:
 *   None
 ****************************************************************************/
void compress_uninit(void);

/****************************************************************************
 * Name: compress_init
 *
 * Description:
 *   Initialize the header 's_header' for this binary
 *
 * Returned value:
 *   OK (0) on Success
 *   ERROR (-1) on Failure
 ****************************************************************************/
int compress_init(int filfd, uint16_t offset, off_t *filelen);

/****************************************************************************
 * Name: compress_read
 *
 * Description:
 *   Read bytes from the compressed file using 'offset' and 'readsize' info
 *   provided for uncompressed file.  The data is read into 'buffer'. Offset
 *   value here is offset from start of uncompressed binary (excluding binary
 *   header).
 *
 * Returned Value:
 *   Number of bytes read into buffer on Success
 *   Negative value on failure
 ****************************************************************************/
int compress_read(int filfd, uint16_t binary_header_size, FAR uint8_t *buffer, size_t readsize, off_t offset);

#endif							/* __INCLUDE_COMPRESS_READ_H */
