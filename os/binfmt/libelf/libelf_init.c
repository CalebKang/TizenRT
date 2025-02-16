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
/****************************************************************************
 * os/binfmt/libelf/libelf_init.c
 *
 *   Copyright (C) 2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <sys/stat.h>

#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <debug.h>
#include <errno.h>

#include <tinyara/fs/fs.h>
#include <tinyara/binfmt/elf.h>

#ifdef CONFIG_COMPRESSED_BINARY
#include <tinyara/binfmt/compression/compress_read.h>
#endif

#include "libelf.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* CONFIG_DEBUG, CONFIG_DEBUG_INFO, and CONFIG_DEBUG_BINFMT have to be
 * defined or CONFIG_ELF_DUMPBUFFER does nothing.
 */

#if !defined(CONFIG_DEBUG_INFO) || !defined(CONFIG_DEBUG_BINFMT)
#undef CONFIG_ELF_DUMPBUFFER
#endif

#ifdef CONFIG_ELF_DUMPBUFFER
#define elf_dumpbuffer(m, b, n) binfodumpbuffer(m, b, n)
#else
#define elf_dumpbuffer(m, b, n)
#endif

/****************************************************************************
 * Private Constant Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: elf_filelen
 *
 * Description:
 *  Get the size of the ELF file
 *
 * Returned Value:
 *   0 (OK) is returned on success and a negated errno is returned on
 *   failure.
 *
 ****************************************************************************/

static inline int elf_filelen(FAR struct elf_loadinfo_s *loadinfo, FAR const char *filename)
{
	struct stat buf;
	int ret;

	/* Get the file stats */

	ret = stat(filename, &buf);
	if (ret < 0) {
		int errval = get_errno();
		berr("Failed to stat file: %d\n", errval);
		return -errval;
	}

	/* Verify that it is a regular file */

	if (!S_ISREG(buf.st_mode)) {
		berr("Not a regular file.  mode: %d\n", buf.st_mode);
		return -ENOENT;
	}

	/* TODO:  Verify that the file is readable.  Not really important because
	 * we will detect this when we try to open the file read-only.
	 */

	/* Return the size of the file in the loadinfo structure */

	loadinfo->filelen = buf.st_size;
	return OK;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: elf_init
 *
 * Description:
 *   This function is called to configure the library to process an ELF
 *   program binary.
 *
 * Returned Value:
 *   0 (OK) is returned on success and a negated errno is returned on
 *   failure.
 *
 ****************************************************************************/

int elf_init(FAR const char *filename, FAR struct elf_loadinfo_s *loadinfo)
{
	int ret;

	binfo("filename: %s loadinfo: %p\n", filename, loadinfo);

	/* Read size in case of FS, size will be passed in load_binary api otherwise */
	if (loadinfo->filelen == 0) {
		/* Get the length of the file. */

		ret = elf_filelen(loadinfo, filename);
		if (ret < 0) {
			berr("elf_filelen failed: %d\n", ret);
			return ret;
		}
	}

	/* Open the binary file for reading (only) */

	loadinfo->filfd = open(filename, O_RDONLY);
	if (loadinfo->filfd < 0) {
		ret = loadinfo->filfd;
		berr("Failed to open ELF binary %s: %d\n", filename, ret);
		return ret;
	}

	if (loadinfo->compression_type > COMPRESS_TYPE_NONE) {
#ifdef CONFIG_COMPRESSED_BINARY
		ret = compress_init(loadinfo->filfd, loadinfo->offset, &loadinfo->filelen);
		if (ret != OK) {
			berr("Failed to read header for compressed binary : %d\n", ret);
			return ret;
		}
#else
		berr("No support for reading compressed binary\n");
		return ERROR;
#endif
	}

	/* Read the ELF ehdr from offset 0 */

	ret = elf_read(loadinfo, (FAR uint8_t *)&loadinfo->ehdr, sizeof(Elf32_Ehdr), 0);
	if (ret < 0) {
		berr("Failed to read ELF header: %d\n", ret);
		return ret;
	}

	elf_dumpbuffer("ELF header", (FAR const uint8_t *)&loadinfo->ehdr, sizeof(Elf32_Ehdr));

	/* Verify the ELF header */

	ret = elf_verifyheader(&loadinfo->ehdr);
	if (ret < 0) {
		/* This may not be an error because we will be called to attempt loading
		 * EVERY binary.  If elf_verifyheader() does not recognize the ELF header,
		 * it will -ENOEXEC whcih simply informs the system that the file is not an
		 * ELF file.  elf_verifyheader() will return other errors if the ELF header
		 * is not correctly formed.
		 */

		berr("Bad ELF header: %d\n", ret);
		return ret;
	}

	return OK;
}
