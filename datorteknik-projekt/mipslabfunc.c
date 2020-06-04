/* mipslabfunc.c
   Originally written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson
   Edited 2020 by David Strömqvist
 */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "Projectheader.h"

/* Declare a helper function which is local to this file */
void updateBlockGameField(void);
void updateStaticGameField(void);

char letterString1[] = "ABCDEFGHIJKLM";
char letterString2[] = "NOPQRSTUVWXYZ";

int8_t arrow[8] = {0x08,0x0c,0x0e,0xff, 0xff, 0x0e,0x0c,0x08};

uint8_t gameBytes[4][60];
int8_t xOffSet = 0;
int8_t yOffSet = 0;

char highScoreString[3][16];

/*
	screenSelect håller reda på vilken skärm som som ska visas.
	0 = spelet
	1 = main skärm
	2 = high score skärm
	3 = game over skärm
	4 = enter name skärm
*/
int8_t screenSelect = 1;

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}


/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/   
void display_debug( volatile int * const addr )
{
  display_string( 1, "Addr" );
  display_string( 2, "Data" );
  num32asc( &textbuffer[1][6], (int) addr );
  num32asc( &textbuffer[2][6], *addr );
  //displayMenu();
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
    DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_image(int x, const uint8_t *data) {
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 32; j++)
			spi_send_recv(~data[i*32 + j]);
	}
}

void displayMenu(void)
{
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

void displayMainMenu(void)
{
	display_string(0, "TETRIS");
	display_string(1, "Spela");
	display_string(2, "High Score");
	display_string(3, "    ");
	
	displayMenu();
}

void highScoreToString()
{
	int i, j;
	char *temp;
	for(i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			highScoreString[i][j] = nameboard[i][j];
		}
		highScoreString[i][3] = ' ';
		temp = itoaconv(scoreboard[i]);
		for (j = 4; j < 16; j++)
		{
			highScoreString[i][j] = *temp;
			temp++;
		}
		temp -= 12;
	}
}

void displayHighScore(void)
{
	highScoreToString();
	display_string(0, "HIGH SCORE");
	display_string(1, highScoreString[0]);
	display_string(2, highScoreString[1]);
	display_string(3, highScoreString[2]);
	
	displayMenu();
}

void displayGameField(int8_t roterat)
{
	if(roterat != 0)
	{
		xOffSet = 0;
		yOffSet = 0;
	}
	int i, j, row;
	updateStaticGameField();
	for(i = 0; i < 4; i++) 
	{
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		spi_send_recv(0xff);
		for(j = 1; j < 128; j++)
		{
			if(j > 61)
			{
				spi_send_recv(0);
			}
			else if (j == 61)
			{
				spi_send_recv(0xff);
			}
			else{
				spi_send_recv(gameBytes[i][j-1]);
			}
		}
	}
}

void updateStaticGameField(void)
{
	int i, j, row;
	for (i = 0; i < 20; i++)
	{
		row = 1;
		for(j = 9; j >= 0; j--)
		{
			row = row << 3;
			if (gamefield[19-i][j] != 0)
				row = row | 0x7;
		}
		row = (row << 1) | 1;
		for (j = 0; j < 4; j++)
		{
			gameBytes[j][i*3]     = row & 0xff;
			gameBytes[j][i*3 + 1] = row & 0xff;
			gameBytes[j][i*3 + 2] = row & 0xff;
			row = row >> 8;
		}
	}
}



void clearScreen()
{
	int i, j;
	for(i = 0; i < 4; i++)
	{
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		for(j = 0; j < 128; j++)
		{
			spi_send_recv(0);
		}
	}
}

/*
	x är offseten -2 om blocket har flyttat höger (+ i x led) 2 om blocket har flyttat vänster (- i x led)
	y är offseten 2 om blocket har flyttat ner 1 steg, -1 om det har stannat med fylld rad
	-2 om fyra rader tagits bort.
	roterat är != 0 om det roterat annars 0
*/
void display_update(int8_t x, int8_t y, int8_t roterat) {
	switch(screenSelect)
	{
		case 0:
			xOffSet = x;
			yOffSet = y;
			displayGameField(roterat); // <----- FEL
			break;
		case 1:
			displayMainMenu();
			break;
		case 2:
			displayHighScore();
			break;
	}
}

/*
	invert and revert the screen based on if number is even or odd.
	revert with even, invert with odd.
*/
void flashForTETRIS(int number)
{
	int command = 0xa6 | (number & 1);
	DISPLAY_CHANGE_TO_COMMAND_MODE;
	spi_send_recv(command);
}

/*
	Print text on on page on the screen.
*/
void printOneLine(int page, char* text)
{
	int j, k;
	int c;
	DISPLAY_CHANGE_TO_COMMAND_MODE;
	spi_send_recv(0x22);
	spi_send_recv(page);
	
	spi_send_recv(0x0);
	spi_send_recv(0x10);
	
	DISPLAY_CHANGE_TO_DATA_MODE;
	
	for(j = 0; j < 16; j++) {
		c = *text;
		text++;
		if(c & 0x80)
			continue;
		
		for(k = 0; k < 8; k++)
			spi_send_recv(font[c*8 + k]);
	}
	
}

void printArrow(int arrowIndex, int oldArrowIndex)
{
	int i, j, k;
	int indexPage = 1;
	if(arrowIndex > 12)
	{
		indexPage = 3;
		arrowIndex -= 13;
		oldArrowIndex -= 13;
	}
	
	for(i = 1; i < 4; i += 2)
	{
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 16; j++) {
			if(j == arrowIndex && indexPage == i)
			{
				for(k = 0; k < 8; k++)
					spi_send_recv(arrow[k]);
			}
			else{
				for(k = 0; k < 8; k++)
					spi_send_recv(0x00);
			}
		}
	}
	
}

char returnLetter(int arrowIndex)
{
	if(arrowIndex < 13)
	{
		return letterString1[arrowIndex];
	}
	else 
		return letterString2[arrowIndex - 13];
}

void enterName()
{
	int lettersEntered = 0;
	int i, j, oldArrowIndex;
	int arrowIndex = 0;
	clearScreen();
	
	printOneLine(0, letterString1);
	printOneLine(2, letterString2);
	
	while (lettersEntered < 3)
	{
		printArrow(arrowIndex, oldArrowIndex);
		
		quicksleep(500000);
		
		if(button_right())
		{
			oldArrowIndex = arrowIndex;
			arrowIndex++;
			if(arrowIndex > 25)
				arrowIndex = 0;
		}
		else if(button_left())
		{
			oldArrowIndex = arrowIndex;
			arrowIndex--;
			if (arrowIndex < 0)
				arrowIndex = 25;
		}
		if (button_rotateleft() || button_rotateright())
		{
			name[lettersEntered] = returnLetter(arrowIndex);
			lettersEntered++;
			flashForTETRIS(1);
			quicksleep(5000);
			flashForTETRIS(0);
		}
			
	}
	return;
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
 void num32asc( char * s, int n ) 
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}


/*
 * itoa
 * 
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 * 
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 * 
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 * 
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 * 
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 * 
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 * 
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 * 
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 12 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";
  
  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}
