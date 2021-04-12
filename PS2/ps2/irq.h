/*
 *  linux/asm/arch/irq.h
 *  By Chenxibing
 *  Copyright (C) 2008 NXP Semiconductors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


extern void get_controller(unsigned int irq, unsigned int *base, unsigned int *irqbit);

extern void lpc32xx_mask_irq(unsigned int irq);

extern void lpc32xx_unmask_irq(unsigned int irq);

extern void lpc32xx_mask_ack_irq(unsigned int irq);

extern int lpc32xx_set_irq_type(unsigned int irq, unsigned int type);

