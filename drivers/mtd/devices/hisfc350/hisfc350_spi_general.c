/*****************************************************************************
 *    Copyright (c) 2009-2011 by HiC
 *    All rights reserved.
 * ***
 *    Create by HiC 2010-09-01
 *
 *****************************************************************************/

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include "../spi_ids.h"
#include "hisfc350.h"
#define DEBUG_SPI 1

/*****************************************************************************/
static int spi_general_wait_ready(struct hisfc_spi *spi)
{
	unsigned long regval;
	unsigned long deadline = jiffies + HISFC350_MAX_READY_WAIT_JIFFIES;
	struct hisfc_host *host = (struct hisfc_host *)spi->host;

	do {
		hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR);
		hisfc_write(host, HISFC350_CMD_CONFIG,
			HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
			| HISFC350_CMD_CONFIG_DATA_CNT(1)
			| HISFC350_CMD_CONFIG_DATA_EN
			| HISFC350_CMD_CONFIG_RW_READ
			| HISFC350_CMD_CONFIG_START);

		HISFC350_CMD_WAIT_CPU_FINISH(host);
		regval = hisfc_read(host, HISFC350_CMD_DATABUF0);
		if (!(regval & SPI_CMD_SR_WIP))
			return 0;

		cond_resched();

	} while (!time_after_eq(jiffies, deadline));

	printk(KERN_ERR "Wait spi flash ready timeout.\n");

	return 1;
}
/*****************************************************************************/
static int spi_general_write_enable(struct hisfc_spi *spi)
{
	unsigned int regval = 0;
	struct hisfc_host *host = (struct hisfc_host *)spi->host;

	hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_WREN);

	regval = HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
		| HISFC350_CMD_CONFIG_START;
	hisfc_write(host, HISFC350_CMD_CONFIG, regval);

	HISFC350_CMD_WAIT_CPU_FINISH(host);

	return 0;
}
/*****************************************************************************/
static int spi_general_entry_4addr(struct hisfc_spi *spi, int enable)
{
	struct hisfc_host *host = (struct hisfc_host *)spi->host;

	if (spi->addrcycle != 4)
		return 0;

	if (enable)
		hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_EN4B);
	else
		hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_EX4B);

	hisfc_write(host, HISFC350_CMD_CONFIG,
		HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
		| HISFC350_CMD_CONFIG_START);

	HISFC350_CMD_WAIT_CPU_FINISH(host);

	host->set_host_addr_mode(host, enable);

	return 0;
}
/*****************************************************************************/
static int spi_general_bus_prepare(struct hisfc_spi *spi, int op)
{
	unsigned int regval = 0;
	struct hisfc_host *host = (struct hisfc_host *)spi->host;

#ifdef HISFCV350_SUPPORT_BUS_WRITE
	regval |= HISFC350_BUS_CONFIG1_WRITE_EN;
#endif
	regval |= HISFC350_BUS_CONFIG1_WRITE_INS(spi->write->cmd);
	regval |= HISFC350_BUS_CONFIG1_WRITE_DUMMY_CNT(spi->write->dummy);
	regval |= HISFC350_BUS_CONFIG1_WRITE_IF_TYPE(spi->write->iftype);

#ifdef HISFCV350_SUPPORT_BUS_READ
	regval |= HISFC350_BUS_CONFIG1_READ_EN;
#endif
	regval |= HISFC350_BUS_CONFIG1_READ_PREF_CNT(0);
	regval |= HISFC350_BUS_CONFIG1_READ_INS(spi->read->cmd);
	regval |= HISFC350_BUS_CONFIG1_READ_DUMMY_CNT(spi->read->dummy);
	regval |= HISFC350_BUS_CONFIG1_READ_IF_TYPE(spi->read->iftype);

	hisfc_write(host, HISFC350_BUS_CONFIG1, regval);
	hisfc_write(host, HISFC350_BUS_CONFIG2,
		HISFC350_BUS_CONFIG2_WIP_LOCATE(0));
	if (op == READ)
		host->set_system_clock(host, spi->read, TRUE);
	else if (op == WRITE)
		host->set_system_clock(host, spi->write, TRUE);

	return 0;
}
/*****************************************************************************/
static int hisfc350_is_quad(struct hisfc_spi *spi)
{
	if (spi->write->iftype == 5 || spi->write->iftype == 6
		|| spi->write->iftype == 7 || spi->read->iftype == 5
		|| spi->read->iftype == 6 || spi->read->iftype == 7) {
		if (DEBUG_SPI)
			printk(KERN_INFO "4r4w SPI...............\n");
		return 1;
	}
	if (DEBUG_SPI)
		printk(KERN_INFO "2r2w or 1r1w SPI...............\n");
	return 0;
}

static int spi_general_qe_enable(struct hisfc_spi *spi)
{
	struct hisfc_host *host = (struct hisfc_host *)spi->host;
	unsigned int regval = 0;
	int count = 0;
	if (hisfc350_is_quad(spi)) {

		if (DEBUG_SPI)
			printk(KERN_INFO "spi_general_qe_enable()\n");
		/* read reg2 */
		hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR2);

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
			printk(KERN_INFO "regval_befor2 : 0x%x\n", regval);
	/* read reg over */
#if 1
	if (!(regval & 0x02)) {
		if (DEBUG_SPI)
			printk(KERN_INFO "Begin to enable QE now---\n");
		do {
			/* write reg start */
			spi->driver->write_enable(spi);
			hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_WRSR);
			hisfc_write(host, HISFC350_CMD_DATABUF0, SPI_CMD_SR_QE);

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

			/*check reg after */
			hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR2);

			hisfc_write(host, HISFC350_CMD_CONFIG,
				HISFC350_CMD_CONFIG_SEL_CS(spi->chipselect)
				| HISFC350_CMD_CONFIG_DATA_CNT(2)
				| HISFC350_CMD_CONFIG_DATA_EN
				| HISFC350_CMD_CONFIG_RW_READ
				| HISFC350_CMD_CONFIG_START);
			HISFC350_CMD_WAIT_CPU_FINISH(host);
			regval = hisfc_read(host, HISFC350_CMD_DATABUF0);

			if (DEBUG_SPI)
				printk(KERN_INFO "regval_after2:0x%x\n",
						regval);
			udelay(1000);
			count++;
		} while (!(regval & SPI_CMD_SR_QE) && (count < 30));
	} else {
		if (DEBUG_SPI)
			printk(KERN_INFO "QE is enable already--\n");
	}
#else
	/* disable qe */
	if (regval & 0x02) {

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

			hisfc_write(host, HISFC350_CMD_INS, SPI_CMD_RDSR2);

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
		} while ((regval & 0x2) && (count < 30));
	} else {
		if (DEBUG_SPI)
			printk(KERN_INFO "QE is disable already--\n");
	}
#endif
	if (count >= 30) {
		printk(KERN_INFO "Enable QE failed\n");
		return -1;
	}
}
	return 0;
}
