/*
 * LFDD - Linux Firmware Debug Driver
 * File: lfdd.h
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
#define LFDD_VERSION            "0.1.0"

#define LFDD_DEFAULT_PATH       "/dev/lfdd"
#define LFDD_MASSBUF_SIZE       256


/*
 * IOCTL commands
 */
#define LFDD_PCI_READ_BYTE      0
#define LFDD_PCI_READ_WORD      1
#define LFDD_PCI_READ_DWORD     2
#define LFDD_PCI_WRITE_BYTE     3
#define LFDD_PCI_WRITE_WORD     4
#define LFDD_PCI_WRITE_DWORD    5
#define LFDD_PCI_READ_256BYTE   6

#define LFDD_MEM_READ_BYTE      7
#define LFDD_MEM_READ_WORD      8
#define LFDD_MEM_READ_DWORD     9
#define LFDD_MEM_WRITE_BYTE     10
#define LFDD_MEM_WRITE_WORD     11
#define LFDD_MEM_WRITE_DWORD    12
#define LFDD_MEM_READ_256BYTE   13

#define LFDD_IO_READ_BYTE       14
#define LFDD_IO_READ_WORD       15
#define LFDD_IO_READ_DWORD      16
#define LFDD_IO_WRITE_BYTE      17
#define LFDD_IO_WRITE_WORD      18
#define LFDD_IO_WRITE_DWORD     19
#define LFDD_IO_READ_256BYTE    20

#define LFDD_I2C_READ_BYTE      21
#define LFDD_I2C_WRITE_BYTE     22

#define LFDD_NVRAM_READ_BYTE    23
#define LFDD_NVRAM_WRITE_BYTE   24

#define LFDD_PCI_ADDR_PORT      0xcf8
#define LFDD_PCI_DATA_PORT      0xcfc


struct lfdd_pci_t {

    unsigned char   bus;
    unsigned char   dev;
    unsigned char   fun;
    unsigned char   reg;

    unsigned int    buf;
    char            mass_buf[ LFDD_MASSBUF_SIZE ];
};


struct lfdd_mem_t {

    unsigned int    addr;

    unsigned int    buf;
    char            mass_buf[ LFDD_MASSBUF_SIZE ];
};


struct lfdd_io_t {

    unsigned int    addr;

    unsigned int    buf;
    char            mass_buf[ LFDD_MASSBUF_SIZE ];
};


#ifdef LFDD_DEBUG
#define pdbg( msg, args... )    printk( KERN_INFO "lfdd %s: " msg, __func__, ##args );
#else
#define pdbg( msg, args... )    do{ }while( 0 ); 
#endif


#ifdef __KERNEL__
unsigned char lfdd_io_read_byte( unsigned int addr );
void lfdd_io_write_byte( unsigned int value, unsigned int addr );
void lfdd_io_read_256byte( struct lfdd_io_t *pio );

unsigned char lfdd_mem_read_byte( unsigned int addr );
unsigned short int lfdd_mem_read_word( unsigned int addr );
unsigned int lfdd_mem_read_dword( unsigned int addr );
void lfdd_mem_write_byte( unsigned int value, unsigned int addr );
void lfdd_mem_write_word( unsigned int value, unsigned int addr );
void lfdd_mem_write_dword( unsigned int value, unsigned int addr );
void lfdd_mem_read_256byte( struct lfdd_mem_t *pmem );

unsigned int lfdd_cal_pci_addr( unsigned char bus, unsigned char dev, unsigned char fun, unsigned char reg );
unsigned char lfdd_pci_read_byte( unsigned int addr );
unsigned short int lfdd_pci_read_word( unsigned int addr );
unsigned int lfdd_pci_read_dword( unsigned int addr );
void lfdd_pci_write_byte( unsigned int value, unsigned int addr );
void lfdd_pci_write_word( unsigned int value, unsigned int addr );
void lfdd_pci_write_dword( unsigned int value, unsigned int addr );
void lfdd_pci_read_256byte( struct lfdd_pci_t *ppci );
#endif


