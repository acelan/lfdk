/*
 * LFDK - Linux Firmware Debug Kit
 * File: libmem.c
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


MemPanel MemScreen;
struct lfdd_mem_t lfdd_mem_data;


extern int x, y;
extern int input;
extern unsigned int counter;
extern int ibuf;
extern char wbuf;
extern char enter_mem;


unsigned int phyaddr = 0;


void WriteMemByteValue( int fd ) {


    lfdd_mem_data.addr = phyaddr + x * 16 + y;
    lfdd_mem_data.buf = wbuf;

    LFDD_IOCTL( fd, LFDD_MEM_WRITE_BYTE, lfdd_mem_data );
}


void ClearMemScreen() {

    DestroyWin( MemScreen, offset );
    DestroyWin( MemScreen, info );
    DestroyWin( MemScreen, value );
    DestroyWin( MemScreen, ascii );
}


void PrintMemScreen( int fd ) {

    int i, j;
    char tmp;


    if( enter_mem ) {


        if( ibuf == 0x0a ) {

            enter_mem = 0;
            return;
        }
        else if ( ((ibuf >= '0') && (ibuf <= '9'))
                || ((ibuf >= 'a') && (ibuf <= 'f'))
                || ((ibuf >= 'A') && (ibuf <= 'F')) ) {


            phyaddr <<= 4;
            phyaddr &= ~0x0f;

            if( ibuf <= '9' ) {

                phyaddr |= (unsigned int)(ibuf - 0x30);
            }
            else if( ibuf > 'F' ) {

                phyaddr |= (unsigned int)(ibuf - 0x60 + 9);
            }
            else {

                phyaddr |= (unsigned int)(ibuf - 0x40 + 9);
            }
        }


    }
    else {

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

            if( (0xffffffff - phyaddr) >= LFDD_MASSBUF_SIZE ) {
        
                phyaddr += LFDD_MASSBUF_SIZE;
            }
            else {
        
                phyaddr = 0;
            }

            input = 0;
        }
        else if( ibuf == KEY_PPAGE ) {

            if( phyaddr >= LFDD_MASSBUF_SIZE ) {
        
                phyaddr -= LFDD_MASSBUF_SIZE;
            }
            else {
        
                phyaddr = (0xffffffff - LFDD_MASSBUF_SIZE + 1);
            }

            input = 0;
        }
        else if( ibuf == 0x0a ) {

            if( input ) {

                input = 0;
                WriteMemByteValue( fd );
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
    }


    //
    // Print Offset Text
    //
    PrintFixedWin( MemScreen, offset, 17, 52, 4, 1, RED_BLUE, "0000 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F0000\n0010\n0020\n0030\n0040\n0050\n0060\n0070\n0080\n0090\n00A0\n00B0\n00C0\n00D0\n00E0\n00F0" );


    //
    // Print memory address
    //
    if( !MemScreen.info ) {

        MemScreen.info = newwin( 1, 47, 22, 0 );
        MemScreen.p_info = new_panel( MemScreen.info );
    }
    wbkgd( MemScreen.info, COLOR_PAIR( WHITE_BLUE ) );
    wattrset( MemScreen.info, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );
    mvwprintw( MemScreen.info, 0, 0, "Type: Memory    Address: " );


    if( enter_mem ) {
    
        if( counter % 2 ) {
                   
            wattrset( MemScreen.info, COLOR_PAIR( YELLOW_RED ) | A_BOLD );
        }
        else {
                   
            wattrset( MemScreen.info, COLOR_PAIR( YELLOW_BLACK ) | A_BOLD );
        }
       
        wprintw( MemScreen.info, "%8.8X", phyaddr );

        counter++;
    }
    else {
    
        wattrset( MemScreen.info, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );
        wprintw( MemScreen.info, "%8.8X", phyaddr );
    }

    
    wattrset( MemScreen.info, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );
    wprintw( MemScreen.info, "h" );
    wattrset( MemScreen.info, A_NORMAL );


    //
    // Read memory space 256 bytes
    //
    if( enter_mem ) {

        memset( lfdd_mem_data.mass_buf, 0xff, LFDD_MASSBUF_SIZE );
    }
    else {

        lfdd_mem_data.addr = phyaddr;
        LFDD_IOCTL( fd, LFDD_MEM_READ_256BYTE, lfdd_mem_data );
    }


    //
    // Print ASCII content
    //
    if( !MemScreen.ascii ) {

        MemScreen.ascii = newwin( 17, 16, 4, 58 );
        MemScreen.p_ascii = new_panel( MemScreen.ascii );
    }


    wbkgd( MemScreen.ascii, COLOR_PAIR( CYAN_BLUE ) );
    wattrset( MemScreen.ascii, COLOR_PAIR( CYAN_BLUE ) | A_BOLD );
    mvwprintw( MemScreen.ascii, 0, 0, "" );

    wprintw( MemScreen.ascii, "0123456789ABCDEF" );
    for( i = 0 ; i < 16 ; i++ ) {

        for( j = 0 ; j < 16 ; j++ ) {

            tmp = ((unsigned char)lfdd_mem_data.mass_buf[ (i * 16) + j ]);
            if( (tmp >= '!') && (tmp <= '~') ) {
            
                wprintw( MemScreen.ascii, "%c", tmp );
            }
            else {

                wprintw( MemScreen.ascii, "." ); 
            }
        }
    }

    wattrset( MemScreen.ascii, A_NORMAL );


    //
    // Print 256bytes content
    //
    if( !MemScreen.value ) {

        MemScreen.value = newwin( 17, 47, 5, 6 );
        MemScreen.p_value = new_panel( MemScreen.value );
    }


    wbkgd( MemScreen.value, COLOR_PAIR( WHITE_BLUE ) );
    mvwprintw( MemScreen.value, 0, 0, "" );


    for( i = 0 ; i < 16 ; i++ ) {

        for( j = 0 ; j < 16 ; j++ ) {
    
                
            //
            // Change Color Pair
            //
            if( y == j && x == i ) {
              
                if( input ) {
                
                    if( counter % 2 ) {
                    
                        wattrset( MemScreen.value, COLOR_PAIR( YELLOW_RED ) | A_BOLD );
                    }
                    else {
                    
                        wattrset( MemScreen.value, COLOR_PAIR( YELLOW_BLACK ) | A_BOLD );
                    }
                    
                    counter++;
                }
                else {

                    wattrset( MemScreen.value, COLOR_PAIR( BLACK_YELLOW ) | A_BOLD ); 
                }
            }
            else if( ((unsigned char)lfdd_mem_data.mass_buf[ (i * 16) + j ]) ) {
           
                wattrset( MemScreen.value, COLOR_PAIR( YELLOW_BLUE ) | A_BOLD );            
            }
            else {
            
                wattrset( MemScreen.value, COLOR_PAIR( WHITE_BLUE ) | A_BOLD );
            }


            //
            // Handle input display
            //
            if( y == j && x == i ) {


                if( input ) {
                
                    wprintw( MemScreen.value, "%2.2X", (unsigned char)wbuf );
                }
                else {
                
                    wprintw( MemScreen.value, "%2.2X", (unsigned char)lfdd_mem_data.mass_buf[ (i * 16) + j ] );
                }
            }
            else {

                wprintw( MemScreen.value, "%2.2X", (unsigned char)lfdd_mem_data.mass_buf[ (i * 16) + j ] );
            }


            //
            // End of color pair
            //
            wattrset( MemScreen.value, A_NORMAL );


            //
            // Move to next byte
            //
            if( j != 15 ) {
          
                wprintw( MemScreen.value, " " );
            }
        }
    }
}


