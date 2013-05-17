/*
 * LFDD - Linux Firmware Debug Driver
 * File: libio.c
 *
 * Copyright (C) 2006 - 2009 Merck Hung <merckhung@gmail.com>
 *										<merck_hung@asus.com.tw>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>

#include <linux/delay.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/highmem.h>

#include "lfdd.h"


static DEFINE_SPINLOCK( status_lock );


unsigned char lfdd_io_read_byte( unsigned int addr ) {

    unsigned long flags;
    char value;

    spin_lock_irqsave( &status_lock, flags );

    value = inb( addr );

    spin_unlock_irqrestore( &status_lock, flags );

    return value;
}


void lfdd_io_write_byte( unsigned int value, unsigned int addr ) {

    unsigned long flags;

    spin_lock_irqsave( &status_lock, flags );

    outb( value, addr );

    spin_unlock_irqrestore( &status_lock, flags );
}


void lfdd_io_read_256byte( struct lfdd_io_t *pio ) { 

    int i;
    unsigned long flags;

    spin_lock_irqsave( &status_lock, flags );

    for( i = 0 ; i < LFDD_MASSBUF_SIZE ; i++ ) {
    
        pio->mass_buf[ i ] = inb( pio->addr + i );
    }

    spin_unlock_irqrestore( &status_lock, flags );
}


