/* mipslabmain.c

   This file written 2015 by Axel Isaksson,
   modified 2015, 2017 by F Lundevall

   Latest update 2017-04-21 by F Lundevall
   
   Edited 2020 by David Strömqvist

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "Projectheader.h"


void *stdout = (void *) 0;
char name[3];

int main(void) {
        /*
	  This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel.
	*/
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
        SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;
	
	display_init();
	quicksleep(100);
	
	
	projinit(); /* Do any lab-specific initialization */
	meminit();
	
	
	
	
	display_string(0, "WELCOME");
	display_string(1, "TO");
	display_string(2, "TETRIS");
	display_string(3, "push any button");
	displayMenu();
	quicksleep(100);
	
	
	ReadToMem();
	screenSelect = 10;
	quicksleep(1000);

	while( 1 )
	{
		
		while(screenSelect == 10)
		{
			if(((PORTD >> 5) & 0xf) >= 1)
			{
				//Initierar random
				srand(TMR2);
				screenSelect = 1;
				  display_update(0,0,0);	
				break;
			}
			quicksleep(3000);
		}
		
		while(screenSelect != 0)
		{
			quicksleep(10000);
			// Main menu
			if(screenSelect == 1){
				if(((PORTD >> 5) & 0x3) == 1){
					screenSelect = 2;
					display_update(0,0,0);
					break;
				}
				else if(((PORTD >> 5) & 0x7) == 4){
					initiategame();
					screenSelect = 0;
					display_update(0,0,0);	
					break;
				}
					  
			}
			
			// High Score Screen
			else if(screenSelect == 2){
				if(((PORTD >> 5) & 0xf) == 2){
					screenSelect = 1;
					display_update(0,0,0);
					break;
				}
			}	

			// Game over screen
			else if(screenSelect == 3)
			{
				quicksleep(10000);
				if(((PORTD >> 5) & 0xf) == 2){
					if (newHighScore(score) == 0)
					{
						screenSelect = 1;
						display_update(0,0,0);
						break;
					}
					else
					{
						enterName();
						highscore( score,  name);
						screenSelect = 2;
						display_update(0,0,0);
						break;
					}
				}
			}
			
		}
		
	}
	return 0;
}
