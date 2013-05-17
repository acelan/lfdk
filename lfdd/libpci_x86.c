/*
 * LFDD - Linux Firmware Debug Driver
 * File: libpci_x86.c
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

#include "lfdd.h"


static DEFINE_SPINLOCK( status_lock );


unsigned int lfdd_cal_pci_addr( unsigned char bus, unsigned char dev, unsigned char fun, unsigned char reg ) {

    unsigned int addr = 0;

    addr |= (bus & 0xff);

    addr <<= 5;
    addr |= (dev & 0x1f);

    addr <<= 3;
    addr |= (fun & 0x07);

    addr <<= 8;
    addr |= 0x80000000;

    addr |= reg;

    return addr;
}


unsigned char lfdd_pci_read_byte( unsigned int addr ) {

    unsigned long flags;
    unsigned int orig_idx;
    unsigned int value;

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Write new address
    outl( addr & 0xfffffffc, LFDD_PCI_ADDR_PORT );

    // Read Value in byte
    value = inl( LFDD_PCI_DATA_PORT );

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );

    value >>= ((addr & 0x03) * 8);

    return (value & 0xff);
}


unsigned short int lfdd_pci_read_word( unsigned int addr ) {

    unsigned long flags;
    unsigned int orig_idx;
    unsigned int value;

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Write new address
    outl( addr & 0xfffffffc, LFDD_PCI_ADDR_PORT );

    // Read Value in byte
    value = inl( LFDD_PCI_DATA_PORT );

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );

    value >>= ((addr & 0x03) * 8);

    return (value & 0xffff);
}


unsigned int lfdd_pci_read_dword( unsigned int addr ) {

    unsigned long flags;
    unsigned int orig_idx;
    unsigned int value;

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Write new address
    outl( addr & 0xfffffffc, LFDD_PCI_ADDR_PORT );

    // Read Value in byte
    value = inl( LFDD_PCI_DATA_PORT );

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );

    return value;
}


void lfdd_pci_write_byte( unsigned int value, unsigned int addr ) {

    unsigned long flags;
    unsigned int orig_idx;
    unsigned int temp;

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Write new address
    outl( addr & 0xfffffffc, LFDD_PCI_ADDR_PORT );

    temp = inl( LFDD_PCI_DATA_PORT );

    value = (value & 0xff) << ((addr & 0x03) * 8);
    temp &= ~(0x000000ff << ((addr & 0x03) * 8));
    temp |= value;

    // Write new Value
    outl( temp, LFDD_PCI_DATA_PORT );

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );
}


void lfdd_pci_write_word( unsigned int value, unsigned int addr ) {

    unsigned long flags;
    unsigned int orig_idx;
    unsigned int temp;

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Write new address
    outl( addr & 0xfffffffc, LFDD_PCI_ADDR_PORT );

    temp = inl( LFDD_PCI_DATA_PORT );

    value = (value & 0xffff) << ((addr & 0x02) * 16);
    temp &= ~(0x0000ffff << ((addr & 0x02) * 16));
    temp |= value;

    // Write new Value
    outl( temp, LFDD_PCI_DATA_PORT );

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );
}


void lfdd_pci_write_dword( unsigned int value, unsigned int addr ) {

    unsigned long flags;
    unsigned int orig_idx;

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Write new address
    outl( addr, LFDD_PCI_ADDR_PORT );

    // Write Value in byte
    outl( value, LFDD_PCI_DATA_PORT );

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );
}


void lfdd_pci_read_256byte( struct lfdd_pci_t *ppci ) { 

    unsigned long flags;
    unsigned int orig_idx;
    int i, value;
    unsigned int addr = lfdd_cal_pci_addr( ppci->bus, ppci->dev, ppci->fun, ppci->reg );

    spin_lock_irqsave( &status_lock, flags );

    // Save original PCI address
    orig_idx = inl( LFDD_PCI_ADDR_PORT );

    // Read Values
    for( i = 0 ; i < LFDD_MASSBUF_SIZE ; i += 4 ) {

        outl( addr + i, LFDD_PCI_ADDR_PORT );
        value = inl( LFDD_PCI_DATA_PORT );

        ppci->mass_buf[ i ]     = (unsigned char) value        & 0xff;
        ppci->mass_buf[ i + 1 ] = (unsigned char)(value >> 8 ) & 0xff;
        ppci->mass_buf[ i + 2 ] = (unsigned char)(value >> 16) & 0xff;
        ppci->mass_buf[ i + 3 ] = (unsigned char)(value >> 24) & 0xff;
    }

    // Restore original PCI address
    outl( orig_idx, LFDD_PCI_ADDR_PORT );

    spin_unlock_irqrestore( &status_lock, flags );
}


