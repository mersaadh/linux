/*
 * sh7724 MMCIF loader
 *
 * Copyright (C) 2010 Magnus Damm
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/platform_data/sh_mmcif.h>
#include <mach/romimage.h>

#define MMCIF_BASE      (void __iomem *)0xa4ca0000

/* MSTPCR2 – Module Stop Control Register 2 */
#define MSTPCR2		0xa4150038

/* PTWCR – Port Write Control Register */
#define PTWCR		0xa4050146

/* PTXCR – Port Transfer Control Register */
#define PTXCR		0xa4050148

/* PSELA – Port Select Register A */
#define PSELA		0xa405014e

/* PSELE – Port Select Register E */
#define PSELE		0xa4050156

/* HIZCRC – High Impedance Control Register */
#define HIZCRC		0xa405015c

/* DRVCRA – Drive Control Register A */
#define DRVCRA		0xa405018a

enum {
	MMCIF_PROGRESS_ENTER,
	MMCIF_PROGRESS_INIT,
	MMCIF_PROGRESS_LOAD,
	MMCIF_PROGRESS_DONE
};

/* SH7724 specific MMCIF loader
 *
 * loads the romImage from an MMC card starting from block 512
 * use the following line to write the romImage to an MMC card
 * # dd if=arch/sh/boot/romImage of=/dev/sdx bs=512 seek=512
 */
asmlinkage void mmcif_loader(unsigned char *buf, unsigned long no_bytes)
{
	mmcif_update_progress(MMCIF_PROGRESS_ENTER);

	/* enable clock to the MMCIF hardware block */
	__raw_writel(__raw_readl(MSTPCR2) & ~0x20000000, MSTPCR2);

	/* setup pins D7-D0 */
	__raw_writew(0x0000, PTWCR);

	/* setup pins MMC_CLK, MMC_CMD */
	__raw_writew(__raw_readw(PTXCR) & ~0x000f, PTXCR);

	/* select D3-D0 pin function */
	__raw_writew(__raw_readw(PSELA) & ~0x2000, PSELA);

	/* select D7-D4 pin function */
	__raw_writew(__raw_readw(PSELE) & ~0x3000, PSELE);

	/* disable Hi-Z for the MMC pins */
	__raw_writew(__raw_readw(HIZCRC) & ~0x0620, HIZCRC);

	/* high drive capability for MMC pins */
	__raw_writew(__raw_readw(DRVCRA) | 0x3000, DRVCRA);

	mmcif_update_progress(MMCIF_PROGRESS_INIT);

	/* setup MMCIF hardware */
	sh_mmcif_boot_init(MMCIF_BASE);

	mmcif_update_progress(MMCIF_PROGRESS_LOAD);

	/* load kernel via MMCIF interface */
	sh_mmcif_boot_do_read(MMCIF_BASE, 512,
	                      (no_bytes + SH_MMCIF_BBS - 1) / SH_MMCIF_BBS,
			      buf);

	/* disable clock to the MMCIF hardware block */
	__raw_writel(__raw_readl(MSTPCR2) | 0x20000000, MSTPCR2);

	mmcif_update_progress(MMCIF_PROGRESS_DONE);
}
