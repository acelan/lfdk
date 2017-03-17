/*
 * LFDK - Linux Firmware Debug Kit
 * File: libpci.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <ncurses.h>
#include <panel.h>

#include "../lfdd/lfdd.h"
#include "lfdk.h"


char read_buffer[ LFDK_MAX_READBUF ];
PCIData lfdd_pci_list[ LFDK_MAX_PCIBUF ];
PCIPanel PCIScreen;
PCILPanel PCILScreen;
struct lfdd_pci_t lfdd_pci_data;


extern int x, y;
extern int curr_index, last_index;
extern int input;
extern unsigned int counter;
extern int ibuf;
extern char wbuf;
extern int func;
extern int maxpcibus;
extern char pciname[ LFDK_MAX_PATH ];


int rpp = 20; // 20 records per page
int lightbar = 0;


int ReadLine( int fd ) {

    char buf;
    int i;
    int tabs;


    memset( read_buffer, 0, LFDK_MAX_READBUF );

    for( i = 0, tabs = 0 ; read( fd, &buf, 1 ) ; ) {

        if( buf == '#' ) {

            //
            // Skip comment
            //
            while( read( fd, &buf, 1 ) && (buf != '\n') );
            return -1;
        }
        else if( buf == '\n' ) {

            if( strlen( read_buffer ) ) {

                return tabs;
            }

            return -1;
        }
        else if( buf == '\t' ) {

            tabs++;
        }
        else {

            read_buffer[ i ] = buf;
            i++;
        }
    }

    return -2;
}


int CompareID( unsigned int id ) {

    int i;
    char idstr[ 5 ];
    char temp[ 5 ];


    //
    // Read ID string
    //
    for( i = 0 ; i < 4 ; i++ ) {

        idstr[ i ] = read_buffer[ i ];
    }
    idstr[ i ] = 0;


    //
    // Convert ASCII to binary
    //
    snprintf( temp, 5, "%4.4x", id );


    return strncmp( temp, idstr, 4 );
}


void GetVendorAndDeviceTexts( int venid, int devid, char *ventxt, char *devtxt ) {

    int fd;
    int tabs;
    int i;
    int done;
    int findven;


    fd = open( pciname, O_RDONLY );
    if( fd < 0 ) {
    
        return;
    }


    for( findven = 0, done = 0 ; ; ) {


        //
        // Parse PCI Database file
        //
        switch( tabs = ReadLine( fd ) ) {

            case 0:
                if( !CompareID( venid ) ) {

                    strncpy( ventxt, (read_buffer + 6), LFDK_MAX_PCINAME );
                    findven = 1;
                }
                break;

            case 1:
                if( findven ) {

                    if( !CompareID( devid ) ) {

                        strncpy( devtxt, (read_buffer + 6), LFDK_MAX_PCINAME );
                        done = 1;
                    }
                }
                break;

            default:
                break;
        }


        if( done ) {

            break;
        }


        //
        // End of File
        //
        if( tabs == -2 ) {

            break;
        }
    }


    close( fd );
}


void ScanPCIDevice( int fd ) {

    int i;
    unsigned char bus = 0, dev = 0, fun = 0;


    //
    // Scan PCI memory space
    //
    last_index = 0;
    for( bus = 0 ; bus < maxpcibus ; bus++ ) {
    
        for( dev = 0 ; dev <= 0x1f ; dev++ ) {
        
            for( fun = 0 ; fun <= 0x07 ; fun++ ) {


                PrintFixedWin( PCILScreen, scan, 1, 36, 10, 25, WHITE_BLUE, "Bus = %2.2X, Dev = %2.2X, Fun = %2.2X", bus, dev, fun );
                update_panels();
                doupdate();

            
                lfdd_pci_data.bus = bus;
                lfdd_pci_data.dev = dev;
                lfdd_pci_data.fun = fun;
                lfdd_pci_data.reg = 0;
                LFDD_IOCTL( fd, LFDD_PCI_READ_WORD, lfdd_pci_data );

                if( (lfdd_pci_data.buf & 0xffff) != 0xffff ) {


                    //
                    // Yes, it's a PCI device
                    //
                    lfdd_pci_list[ last_index ].bus   = bus;
                    lfdd_pci_list[ last_index ].dev   = dev;
                    lfdd_pci_list[ last_index ].fun   = fun;
                    lfdd_pci_list[ last_index ].venid = (unsigned short int)lfdd_pci_data.buf;

                    
                    //
                    // Read and record Device ID
                    //
                    lfdd_pci_data.reg += 0x02;
                    LFDD_IOCTL( fd, LFDD_PCI_READ_WORD, lfdd_pci_data );
                    lfdd_pci_list[ last_index ].devid = (unsigned short int)lfdd_pci_data.buf;


                    //
                    // Get Texts
                    //
                    GetVendorAndDeviceTexts( lfdd_pci_list[ last_index ].venid, lfdd_pci_list[ last_index ].devid
                                            ,lfdd_pci_list[ last_index ].ventxt, lfdd_pci_list[ last_index ].devtxt );


                    //
                    // Move to next record
                    //
                    last_index++;
                }
            }
        }
    }


    DestroyWin( PCILScreen, scan );
}


void WritePCIByteValue( int fd ) {


    lfdd_pci_data.bus = lfdd_pci_list[ curr_index ].bus;
    lfdd_pci_data.dev = lfdd_pci_list[ curr_index ].dev;
    lfdd_pci_data.fun = lfdd_pci_list[ curr_index ].fun;
    lfdd_pci_data.reg = x * 16 + y;
    lfdd_pci_data.buf = wbuf;

    LFDD_IOCTL( fd, LFDD_PCI_WRITE_BYTE, lfdd_pci_data );
}


void ClearPCIScreen() {

    DestroyWin( PCIScreen, ven );
    DestroyWin( PCIScreen, dev );
    DestroyWin( PCIScreen, offset );
    DestroyWin( PCIScreen, info );
    DestroyWin( PCIScreen, rtitle );
    DestroyWin( PCIScreen, value );
}


void PrintPCIScreen( int fd ) {

    int i, j;


    if( ibuf == KEY_UP ) {

        if( x > 0 ) {

            x--;
        }

        input = 0;
    }
    else if( ibuf == KEY_DOWN ) {

        if( x < 15 ) {

            x++;
        }

        input = 0;
    }
    else if( ibuf == KEY_LEFT ) {

        if( y > 0 ) {

            y--;
        }

        input = 0;
    }
    else if( ibuf == KEY_RIGHT ) {

        if( y < 15 ) {

            y++;
        }

        input = 0;
    }
    else if( ibuf == KEY_NPAGE ) {

        if( (curr_index + 1) < last_index ) {

            curr_index++;
        }
        else {

            curr_index = 0;
        }

        input = 0;
    }
    else if( ibuf == KEY_PPAGE ) {

        if( curr_index > 0 ) {

            curr_index--;
        }
        else {

            curr_index = last_index - 1;
        }

        input = 0;
    }
    else if( ibuf == 0x0a ) {

        if( input ) {

            input = 0;
            WritePCIByteValue( fd );
        }
    }
    else if ( ((ibuf >= '0') && (ibuf <= '9'))
                || ((ibuf >= 'a') && (ibuf <= 'f'))
                || ((ibuf >= 'A') && (ibuf <= 'F')) ) {

        if( !input ) {

            wbuf = 0x00;
            input = 1;
        }


        wbuf <<= 4;
        wbuf &= 0xf0;


        if( ibuf <= '9' ) {

            wbuf |= ibuf - 0x30;
        }
        else if( ibuf > 'F' ) {

            wbuf |= ibuf - 0x60 + 9;
        }
        else {

            wbuf |= ibuf - 0x40 + 9;
        }
    }


    //
    // Print Device Name
    //
    PrintFixedWin( PCIScreen, ven, 1, 70, 1, 1, CYAN_BLUE, "%70s", " " );
    if( lfdd_pci_list[ curr_index ].ventxt[ 0 ] ) {

        PrintFixedWin( PCIScreen, ven, 1, 70, 1, 1, CYAN_BLUE, "Vendor: %.62s", lfdd_pci_list[ curr_index ].ventxt );
    }
    else {

        PrintFixedWin( PCIScreen, ven, 1, 70, 1, 1, CYAN_BLUE, "Unknown Vendor" );
    }


    PrintFixedWin( PCIScreen, dev, 1, 70, 2, 1, CYAN_BLUE, "%70s", " " );
    if( lfdd_pci_list[ curr_index ].devtxt[ 0 ] ) {

        PrintFixedWin( PCIScreen, dev, 1, 70, 2, 1, CYAN_BLUE, "Device: %.62s", lfdd_pci_list[ curr_index ].devtxt );
    }
    else {
    
        PrintFixedWin( PCIScreen, dev, 1, 70, 2, 1, CYAN_BLUE, "Unknown Device" );
    }


    //
    // Print Offset Text
    //
    PrintFixedWin( PCIScreen, offset, 17, 52, 4, 1, RED_BLUE, "0000 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F0000\n0010\n0020\n0030\n0040\n0050\n0060\n0070\n0080\n0090\n00A0\n00B0\n00C0\n00D0\n00E0\n00F0" );


    //
    // Print PCI bus, device, function number
    //
    PrintFixedWin( PCIScreen, info, 1, 47, 22, 0, WHITE_BLUE, "Type: PCI    Bus %2.2X    Device %2.2X    Function %2.2X"
                , lfdd_pci_list[ curr_index ].bus, lfdd_pci_list[ curr_index ].dev, lfdd_pci_list[ curr_index ].fun );


    //
    // Read PCI configuration space 256 bytes
    //
    lfdd_pci_data.bus = lfdd_pci_list[ curr_index ].bus;
    lfdd_pci_data.dev = lfdd_pci_list[ curr_index ].dev;
    lfdd_pci_data.fun = lfdd_pci_list[ curr_index ].fun;
    lfdd_pci_data.reg = 0;
    LFDD_IOCTL( fd, LFDD_PCI_READ_256BYTE, lfdd_pci_data );



    //
    // Print PCI information
    //
    if( !PCIScreen.rtitle ) {

        PCIScreen.rtitle = newwin( 17, 24, 4, 56 );
        PCIScreen.p_rtitle = new_panel( PCIScreen.rtitle );
    }

    mvwprintw( PCIScreen.rtitle, 0, 0, "" );
    wbkgd( PCIScreen.rtitle, COLOR_PAIR( CYAN_BLUE ) );
    wattrset( PCIScreen.rtitle, COLOR_PAIR( CYAN_BLUE ) | A_BOLD );

    wprintw( PCIScreen.rtitle, "Refresh    : ON\n\n" );
    wprintw( PCIScreen.rtitle, "Data Width : 8 bits\n\n" );

    wprintw( PCIScreen.rtitle, "VID:DID = %4.4X:%4.4X\n", lfdd_pci_list[ curr_index ].venid, lfdd_pci_list[ curr_index ].devid );
    wprintw( PCIScreen.rtitle, "Rev ID        : %2.2X\n", (unsigned char)lfdd_pci_data.mass_buf[ 0x08 ] );
    wprintw( PCIScreen.rtitle, "Int Line (IRQ): %2.2X\n", (unsigned char)lfdd_pci_data.mass_buf[ 0x3c ] );
    wprintw( PCIScreen.rtitle, "Int Pin       : %2.2X\n\n", (unsigned char)lfdd_pci_data.mass_buf[ 0x3c + 8 ] );

    wprintw( PCIScreen.rtitle, "Mem: 00000000 00000000\n" );
    wprintw( PCIScreen.rtitle, "Mem: 00000000 00000000\n" );
    wprintw( PCIScreen.rtitle, "Mem: 00000000 00000000\n" );
    wprintw( PCIScreen.rtitle, "Mem: 00000000 00000000\n" );
    wprintw( PCIScreen.rtitle, "Mem: 00000000 00000000\n" );
    wprintw( PCIScreen.rtitle, "Mem: 00000000 00000000\n\n" );

    wprintw( PCIScreen.rtitle, "ROM: 00000000\n" );
    wattrset( PCIScreen.rtitle, A_NORMAL );



    //
    // Print 256bytes PCI Configuration Space
    //
    if( !PCIScreen.value ) {

        PCIScreen.value = newwin( 17, 47, 5, 6 );
        PCIScreen.p_value = new_panel( PCIScreen.value );
    }


    wbkgd( PCIScreen.value, COLOR_PAIR( WHITE_BLUE ) );
    mvwprintw( PCIScreen.value, 0, 0, "" );


    for( i = 0 ; i < 16 ; i ++ ) {

        for( j = 0 ; j < 16 ; j++ ) {
    
                
            //
            // Change Color Pair
            //
            if( y == j && x == i ) {
              
                if( input ) {
                
                    if( counter % 2 ) {
                    
                        wattrset( PCIScreen.value, COLOR_PAIR( YELLOW_RED ) | A_BOLD );
                    }
                    else {
                    
                        wattrset( PCIScreen.value, COLOR_PAIR( YELLOW_BLACK ) | A_BOLD );
                    }
                    
                    counter++;
                }
                else {

                    wattrset( PCIScreen.value, COLOR_PAIR( BLACK_YELLOW ) | A_BOLD ); 
                }
            }
            else if( ((unsigned char)lfdd_pci_data.mass_buf[ (i * 16) + j ]) ) {
           
                wattrset( PCIScreen.value, COLOR_PAIR( YELLOW_BLUE ) | A_BOLD );            
            }
            else {
            
                wattrset( PCIScreen.value, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );
            }


            //
            // Handle input display
            //
            if( y == j && x == i ) {


                if( input ) {
                
                    wprintw( PCIScreen.value, "%2.2X", (unsigned char)wbuf );
                }
                else {
                
                    wprintw( PCIScreen.value, "%2.2X", (unsigned char)lfdd_pci_data.mass_buf[ (i * 16) + j ] );
                }
            }
            else {

                wprintw( PCIScreen.value, "%2.2X", (unsigned char)lfdd_pci_data.mass_buf[ (i * 16) + j ] );
            }


            //
            // End of color pair
            //
            wattrset( PCIScreen.value, A_NORMAL );


            //
            // Move to next byte
            //
            if( j != 15 ) {
          
                wprintw( PCIScreen.value, " " );
            }
        }
    }
}


void ClearPCILScreen( void ) {

    DestroyWin( PCILScreen, title );
    DestroyWin( PCILScreen, devname );
    DestroyWin( PCILScreen, vendev );
}


void PrintPCILScreen( void ) {

    int i;


    //
    // Adjust Light Bar and curr_index
    //
    if( (curr_index + rpp) > last_index ) {
    
        curr_index = last_index - rpp;
        if( curr_index < 0 ) {
        
            curr_index = 0;
        }
    }


    if( ibuf == KEY_UP ) {

        if( lightbar > 0 ) {

            lightbar--;
        }
        else {
        
            lightbar = 0;
            if( curr_index > 0 ) {
            
                curr_index--;
            }
        }
    }
    else if( ibuf == KEY_DOWN ) {

        if( last_index > rpp ) {
        
            if( (lightbar + 1) < rpp ) {
        
                lightbar++;
            }
            else {
        
                lightbar = rpp - 1;
                if( (last_index - curr_index - 1) >= rpp ) {

                    curr_index++; 
                }
            }
        }
        else {
        
            if( (lightbar + 1) < last_index ) {

                lightbar++;
            }
        }
    }
    else if( ibuf == KEY_NPAGE ) {

        if( (curr_index + rpp) <= (last_index - rpp) ) {

            curr_index += rpp;
        }
        else {

            curr_index = last_index - rpp;
        }

        lightbar = 0;
    }
    else if( ibuf == KEY_PPAGE ) {

        if( (curr_index - rpp) >= 0 ) {

            curr_index -= rpp;
        }
        else {

            curr_index = 0;
        }

        lightbar = 0;
    }
    else if( ibuf == 0x0a ) {

        curr_index += lightbar;
        func = PCI_DEVICE_FUNC;
        ClearPCILScreen();
        goto pcil_done;
    }


    //
    // Print Title
    //
    PrintFixedWin( PCILScreen, title, 1, 80, 1, 0, BLACK_GREEN, "Name                                              Vendor  Device  Bus# Dev# Fun#" );


    //
    // Print PCI device name
    //
    if( !PCILScreen.devname ) {

        PCILScreen.devname = newwin( 20, 50, 2, 0 );
        PCILScreen.p_devname = new_panel( PCILScreen.devname );
    }


    wbkgd( PCILScreen.devname, COLOR_PAIR( WHITE_BLUE ) );


    mvwprintw( PCILScreen.devname, 0, 0, "" );
    for( i = curr_index ; ((i < (curr_index + rpp)) && (i < last_index)) ; i++ ) {

        if( (i - curr_index) == lightbar ) {
        
            wattrset( PCILScreen.devname, COLOR_PAIR( BLACK_YELLOW ) | A_BOLD );
        }
        else {
        
            wattrset( PCILScreen.devname, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );
        }


        if( !lfdd_pci_list[ i ].ventxt[ 0 ] ) {
            
            wprintw( PCILScreen.devname, "%4.4X, ", lfdd_pci_list[ i ].venid );
        }
        else {
            
            wprintw( PCILScreen.devname, "%.12s, ", lfdd_pci_list[ i ].ventxt );
        }
            
        if( !lfdd_pci_list[ i ].devtxt[ 0 ] ) {
           
            wprintw( PCILScreen.devname, "%4.4X\n", lfdd_pci_list[ i ].devid );
        }
        else {
            
            wprintw( PCILScreen.devname, "%.35s\n", lfdd_pci_list[ i ].devtxt );
        }


        wattrset( PCILScreen.devname, A_NORMAL );
    }


    if( !PCILScreen.vendev ) {

        PCILScreen.vendev = newwin( 20, 30, 2, 50 );
        PCILScreen.p_vendev = new_panel( PCILScreen.vendev );
    }


    wbkgd( PCILScreen.vendev, COLOR_PAIR( WHITE_BLUE ) );
    wattrset( PCILScreen.vendev, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );


    mvwprintw( PCILScreen.vendev, 0, 0, "" );
    for( i = curr_index ; ((i < (curr_index + rpp)) && (i < last_index)) ; i++ ) {

        wprintw( PCILScreen.vendev, "%4.4X    %4.4X     %2.2X   %2.2X   %2.2X\n", lfdd_pci_list[ i ].venid, lfdd_pci_list[ i ].devid, lfdd_pci_list[ i ].bus, lfdd_pci_list[ i ].dev, lfdd_pci_list[ i ].fun );
    }


    wattrset( PCILScreen.vendev, A_NORMAL );

pcil_done:
    return;
}


