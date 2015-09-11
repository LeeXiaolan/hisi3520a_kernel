/******************************************************************************
*    Copyright (c) 2009-2012 by Hisi.
*    All rights reserved.
******************************************************************************/

#include <linux/io.h>
#include <linux/mtd/mtd.h>
#include <linux/errno.h>

#include "../spi_ids.h"
#include "hisfc350.h"

#define SPI_BRWR	0x17
#define SPI_EN4B	0x80
#define SPI_EX4B	0x00


static int spi_mx25l25635e_qe_enable(struct hisfc_spi *spi)
{
	struct hisfc_host *host = (struct hisfc_host *)spi->host;
	unsigned int regval = 0;
	int count = 0;
	if (hisfc350_is_quad(spi)) {

		if (DEBUG_SPI)
			printk(KERN_INFO "spi_mx25l25635e_qe_enable()\n");
		hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR);

		hisfc_write(host, HISFC350_CMD_CONFIG,
			HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
			| HISFC350_CMD_CONFIG_DATA_CNT(2)
			| HISFC350_CMD_CONFIG_DATA_EN
			| HISFC350_CMD_CONFIG_RW_READ
			| HISFC350_CMD_CONFIG_START);

		HISFC350_CMD_WAIT_CPU_FINISH(host);

		if (DEBUG_SPI)
			printk(KERN_INFO "regval_init : 0x%x\n", regval);

		regval = hisfc_read(host, HISFC350_CMD_DATABUF0);

		if (DEBUG_SPI)
			printk(KERN_INFO "regval_befor2 = 40?: 0x%x\n", regval);
	}

#if 1
	if (!(regval & 0x40)) {
		if (DEBUG_SPI)
			printk(KERN_INFO "Begin to enable QE now---\n");
		do {
			spi->driver->write_enable(spi);
			hisfc_write(host, HISFC350_CMD_INS, 0x01);
			hisfc_write(host, HISFC350_CMD_DATABUF0, 0x40);

			hisfc_write(host, HISFC350_CMD_CONFIG,
				HISFC350_CMD_CONFIG_MEM_IF_TYPE(spi->
					write->iftype)
				| HISFC350_CMD_CONFIG_DATA_CNT(1)
				| HISFC350_CMD_CONFIG_DATA_EN
				| HISFC350_CMD_CONFIG_DUMMY_CNT(spi->
					write->dummy)
				| HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
				| HISFC350_CMD_CONFIG_START);

			HISFC350_CMD_WAIT_CPU_FINISH(host);

			hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR);

			hisfc_write(host, HISFC350_CMD_CONFIG,
				HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
				| HISFC350_CMD_CONFIG_DATA_CNT(1)
				| HISFC350_CMD_CONFIG_DATA_EN
				| HISFC350_CMD_CONFIG_RW_READ
				| HISFC350_CMD_CONFIG_START);
			HISFC350_CMD_WAIT_CPU_FINISH(host);
			regval = hisfc_read(host, HISFC350_CMD_DATABUF0);

			if (DEBUG_SPI)
				printk(KERN_INFO "regval_after2 : 0x%x\n",
						regval);
			udelay(1000);
			count++;
		} while (!(regval & 0x40) && (count < 30));

	} else {
		if (DEBUG_SPI)
			printk(KERN_INFO "QE is enable already--\n");
	}
#else

	if (regval & 0x40) {

		if (DEBUG_SPI)
			printk(KERN_INFO "Begin to disable QE now---\n");

		do {

			spi->driver->write_enable(spi);
			hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_WRSR);
			hisfc_write(host, HISFC350_CMD_DATABUF0,
				SPI_CMD_SR_XQE);

			hisfc_write(host, HISFC350_CMD_CONFIG,
				HISFC350_CMD_CONFIG_MEM_IF_TYPE(spi->
					write->iftype)
				| HISFC350_CMD_CONFIG_DATA_CNT(2)
				| HISFC350_CMD_CONFIG_DATA_EN
				| HISFC350_CMD_CONFIG_DUMMY_CNT(spi->
					write->dummy)
				| HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
				| HISFC350_CMD_CONFIG_START);

			HISFC350_CMD_WAIT_CPU_FINISH(host);

			hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR);

			hisfc_write(host, HISFC350_CMD_CONFIG,
				HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
				| HISFC350_CMD_CONFIG_DATA_CNT(2)
				| HISFC350_CMD_CONFIG_DATA_EN
				| HISFC350_CMD_CONFIG_RW_READ
				| HISFC350_CMD_CONFIG_START);
			HISFC350_CMD_WAIT_CPU_FINISH(host);
			regval = hisfc_read(host, HISFC350_CMD_DATABUF0);

			if (DEBUG_SPI)
				printk(KERN_INFO "regval_after2 : 0x%x\n",
						regval);
			udelay(1000);
			count++;
		} while ((regval & 0x40) && (count < 30));

	} else {
		if (DEBUG_SPI)
			printk(KERN_INFO "QE is disable already--\n");
	}
#endif
	if (count >= 30) {
		printk(KERN_INFO "Enable QE failed\n");
		return -1;
	}

	return 0;
}

