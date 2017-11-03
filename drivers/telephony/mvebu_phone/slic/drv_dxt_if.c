/*******************************************************************************
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * This software file (the "File") is owned and distributed by Marvell
 * International Ltd. and/or its affiliates ("Marvell") under the following
 * alternative licensing terms.  Once you have made an election to distribute the
 * File under one of the following license alternatives, please (i) delete this
 * introductory statement regarding license alternatives, (ii) delete the three
 * license alternatives that you have not elected to use and (iii) preserve the
 * Marvell copyright notice above.
 *
 * ********************************************************************************
 * Marvell Commercial License Option
 *
 * If you received this File from Marvell and you have entered into a commercial
 * license agreement (a "Commercial License") with Marvell, the File is licensed
 * to you under the terms of the applicable Commercial License.
 *
 * ********************************************************************************
 * Marvell GPL License Option
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ********************************************************************************
 * Marvell GNU General Public License FreeRTOS Exception
 *
 * If you received this File from Marvell, you may opt to use, redistribute and/or
 * modify this File in accordance with the terms and conditions of the Lesser
 * General Public License Version 2.1 plus the following FreeRTOS exception.
 * An independent module is a module which is not derived from or based on
 * FreeRTOS.
 * Clause 1:
 * Linking FreeRTOS statically or dynamically with other modules is making a
 * combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 * As a special exception, the copyright holder of FreeRTOS gives you permission
 * to link FreeRTOS with independent modules that communicate with FreeRTOS solely
 * through the FreeRTOS API interface, regardless of the license terms of these
 * independent modules, and to copy and distribute the resulting combined work
 * under terms of your choice, provided that:
 * 1. Every copy of the combined work is accompanied by a written statement that
 * details to the recipient the version of FreeRTOS used and an offer by yourself
 * to provide the FreeRTOS source code (including any modifications you may have
 * made) should the recipient request it.
 * 2. The combined work is not itself an RTOS, scheduler, kernel or related
 * product.
 * 3. The independent modules add significant and primary functionality to
 * FreeRTOS and do not merely extend the existing functionality already present in
 * FreeRTOS.
 * Clause 2:
 * FreeRTOS may not be used for any competitive or comparative purpose, including
 * the publication of any form of run time or compile time metric, without the
 * express permission of Real Time Engineers Ltd. (this is the norm within the
 * industry and is intended to ensure information accuracy).
 *
 * ********************************************************************************
 * Marvell BSD License Option
 *
 * If you received this File from Marvell, you may opt to use, redistribute and/or
 * modify this File under the following licensing terms.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *	* Redistributions of source code must retain the above copyright notice,
 *	  this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *
 *	* Neither the name of Marvell nor the names of its contributors may be
 *	  used to endorse or promote products derived from this software without
 *	  specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "drv_dxt_if.h"

static int drv_dxt_spi_cs;
static int drv_dxt_irq;
static void *drv_dxt_irq_dev;
static irq_handler_t drv_dxt_irq_handler;

void drv_dxt_if_signal_interrupt(void)
{
	if (drv_dxt_irq_handler == NULL)
		return;

	drv_dxt_irq_handler(drv_dxt_irq, drv_dxt_irq_dev);
}

void drv_dxt_if_enable_irq(u32 irq)
{
	/* We have only one TDM channel */
	mv_phone_intr_enable(0);
}
EXPORT_SYMBOL(drv_dxt_if_enable_irq);

void drv_dxt_if_disable_irq(u32 irq)
{
	/* We have only one TDM channel */
	mv_phone_intr_disable(0);
}
EXPORT_SYMBOL(drv_dxt_if_disable_irq);

int drv_dxt_if_request_irq(u32 irq, irq_handler_t handler, u64 flags,
			   const char *name, void *dev)
{
	drv_dxt_irq = irq;
	drv_dxt_irq_dev = dev;
	drv_dxt_irq_handler = handler;

	return 0;
}
EXPORT_SYMBOL(drv_dxt_if_request_irq);

void drv_dxt_if_free_irq(u32 irq, void *dev)
{
	drv_dxt_irq_handler = NULL;
}
EXPORT_SYMBOL(drv_dxt_if_free_irq);

void drv_dxt_if_spi_cs_set(u32 dev_no, u32 hi_lo)
{
	if (hi_lo == 0)
		drv_dxt_spi_cs = dev_no;
	else
		drv_dxt_spi_cs = -1;
}
EXPORT_SYMBOL(drv_dxt_if_spi_cs_set);

int drv_dxt_if_spi_ll_read_write(u8 *tx_data, u32 tx_size,
				 u8 *rx_data, u32 rx_size)
{
	uint16_t *ptr;
	int i;

	if ((tx_size & 1) || (rx_size & 1)) {
		pr_err("drv_dxt_if: SPI transfer is not word aligned!\n");
		return 0;
	}

	ptr = (uint16_t *)tx_data;
	for (i = 0; i < tx_size / 2; i++, ptr++)
		*ptr = htons(*ptr);

	if (rx_data != NULL && rx_size != 0) {
		mv_phone_spi_read(drv_dxt_spi_cs, tx_data, tx_size,
				  rx_data, rx_size, SPI_TYPE_SLIC_LANTIQ);
	} else if (tx_data != NULL && tx_size > 2) {
		mv_phone_spi_write(drv_dxt_spi_cs, tx_data, 2, tx_data + 2,
				   tx_size - 2, SPI_TYPE_SLIC_LANTIQ);
	} else {
		pr_err("drv_dxt_if: Unsupported SPI access mode!\n");
	}

	ptr = (uint16_t *)rx_data;
	for (i = 0; i < rx_size / 2; i++, ptr++)
		*ptr = htons(*ptr);

	return 0;
}
EXPORT_SYMBOL(drv_dxt_if_spi_ll_read_write);
