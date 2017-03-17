/*
 * LFDK - Linux Firmware Debug Kit
 * File: lfdk.c
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
#include <time.h>

#include <ncurses.h>
#include <panel.h>

#include "../lfdd/lfdd.h"
#include "lfdk.h"
#include "libpci.h"
#include "libmem.h"
#include "libio.h"

static const char *progname;
static BasePanel BaseScreen;


int x = 0, y = 0;
int curr_index = 0, last_index;
int input = 0;
unsigned int counter = 0;
int ibuf;
char wbuf;
int func = PCI_DEVICE_FUNC;
int maxpcibus = 255;
char pciname[ LFDK_MAX_PATH ];
char enter_mem = 0;


void PrintBaseScreen( void );


static void usage( void ) {

    fprintf( stderr, "\n"LFDK_VERTEXT"\n" );
	fprintf( stderr, "Copyright (C) 2006 - 2009, Merck Hung <merckhung@gmail.com>\n" );
    fprintf( stderr, "Usage: "LFDK_PROGNAME" [-h] [-d /dev/lfdd] [-n ./pci.ids] [-b 255]\n" );
    fprintf( stderr, "\t-n\tFilename of PCI Name Database, default is /usr/share/misc/pci.ids\n" );
    fprintf( stderr, "\t-d\tDevice name of Linux Firmware Debug Driver, default is /dev/lfdd\n" );
    fprintf( stderr, "\t-b\tMaximum PCI Bus number to scan, default is 255\n" );
    fprintf( stderr, "\t-h\tprint this message.\n");
    fprintf( stderr, "\n");
}


void InitColorPairs( void ) {

    init_pair( WHITE_RED, COLOR_WHITE, COLOR_RED );
    init_pair( WHITE_BLUE, COLOR_WHITE, COLOR_BLUE );
    init_pair( BLACK_WHITE, COLOR_BLACK, COLOR_WHITE ); 
    init_pair( CYAN_BLUE, COLOR_CYAN, COLOR_BLUE );
    init_pair( RED_BLUE, COLOR_RED, COLOR_BLUE );
    init_pair( YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE );
    init_pair( BLACK_GREEN, COLOR_BLACK, COLOR_GREEN );
    init_pair( BLACK_YELLOW, COLOR_BLACK, COLOR_YELLOW );
    init_pair( YELLOW_RED, COLOR_YELLOW, COLOR_RED );
    init_pair( YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK );
    init_pair( WHITE_YELLOW, COLOR_WHITE, COLOR_YELLOW );
}


void PrintBaseScreen( void ) {


    //
    // Background Color
    //
    PrintWin( BaseScreen, bg, 23, 80, 0, 0, WHITE_BLUE, "" );


    //
    // Base Screen
    //
    PrintWin( BaseScreen, logo, 1, 80, 0, 0, WHITE_RED, "Linux Firmware Debug Kit "LFDK_VERSION );
    PrintWin( BaseScreen, copyright, 1, 32, 0, 48, WHITE_RED, "Merck Hung <merckhung@gmail.com>" );
    PrintWin( BaseScreen, help, 1, 80, 23, 0, BLACK_WHITE, "(Q)uit (L)PCI List (M)Memory (I)I/O" );


    update_panels();
    doupdate();
}


int main( int argc, char **argv ) {

    extern char *optarg;
    extern int optind;

    char c, device[ LFDK_MAX_PATH ];
    int i, fd, orig_fl;

    struct tm *nowtime;
    time_t timer;
    int last_sec;


    //
    // Initialize & set default value
    //
    strncpy( device, LFDD_DEFAULT_PATH, LFDK_MAX_PATH );
    strncpy( pciname, LFDK_DEFAULT_PCINAME, LFDK_MAX_PATH );


    while( (c = getopt( argc, argv, "b:n:d:h" )) != EOF ) {

        switch( c ) {
        

            //
            // Change default path of device
            //
            case 'd' :

                strncpy( device, optarg, LFDK_MAX_PATH );
                break;

            //
            // Change default path of PCI name database
            //
            case 'n' :

                strncpy( pciname, optarg, LFDK_MAX_PATH );
                break;

            case 'b' :

                maxpcibus = atoi( optarg );
                if( maxpcibus >= 256 ) {
                
                    fprintf( stderr, "Maximum PCI Bus value must be less than 256\n" );
                    return 1;
                }
                break;

            case 'v' :
                break;

            case 'h' :
            default:
                usage();
                return 1;
        }
    }


    //
    // Start communicate with LFDD I/O control driver
    //
    fd = open( device, O_RDWR );
    if( fd < 0 ) {

        fprintf( stderr, "Cannot open device: %s\n", device );
        return 1;
    }


    //
    // Ncurse start
    //
    initscr();
    start_color();
    cbreak();
    noecho();
    nodelay( stdscr, 1 );
    keypad( stdscr, 1 );
    curs_set( 0 );


    //
    // Initialize color pairs for later use
    //
    InitColorPairs();


    //
    // Prepare Base Screen
    //
    PrintBaseScreen();


    //
    // Scan PCI devices
    //
    ScanPCIDevice( fd );


    for( ; ; ) {


        ibuf = getch();
        if( (ibuf == 'q') || (ibuf == 'Q') ) {
        
            //
            // Exit when ESC pressed
            //
            break;
        }


        //
        // Major function switch key binding
        //
        if( (ibuf == 'l') || (ibuf == 'L') ) {

            func = PCI_LIST_FUNC;
            ClearPCIScreen();
            ClearMemScreen(); 
            ClearIOScreen();
            continue;
        }
        else if( (ibuf == 'm') || (ibuf == 'M') ) {
        
            enter_mem = 1;
            func = MEM_SPACE_FUNC;
            ClearPCIScreen();
            ClearPCILScreen();
            ClearIOScreen();
            continue;
        }
        else if( (ibuf == 'i') || (ibuf == 'I') ) {

            enter_mem = 1;
            func = IO_SPACE_FUNC;
            ClearPCIScreen();
            ClearPCILScreen();
            ClearMemScreen();
            continue;
        }


        //
        // Update timer
        //
        time( &timer );
        nowtime = localtime( &timer );
        last_sec = nowtime->tm_sec;


        // Skip redundant update of timer
        if( last_sec == nowtime->tm_sec ) {

            PrintFixedWin( BaseScreen, time, 1, 8, 23, 71, BLACK_WHITE, "%2.2d:%2.2d:%2.2d", nowtime->tm_hour, nowtime->tm_min,  nowtime->tm_sec );
        }


        //
        // Major Functions
        //
        switch( func ) {
        
            case PCI_DEVICE_FUNC:

                PrintPCIScreen( fd );
                break;

            case PCI_LIST_FUNC:

                PrintPCILScreen();
                break;

            case MEM_SPACE_FUNC:

                PrintMemScreen( fd );
                break;

            case IO_SPACE_FUNC:

                PrintIOScreen( fd );
                break;

            default:
                break;
        } 


        //
        // Refresh Screen
        //
        update_panels();
        doupdate();


        usleep( 1000 );
    }


    endwin();
    close( fd );


    fprintf( stderr, "\n" );
    usage();


    return 0;
}


