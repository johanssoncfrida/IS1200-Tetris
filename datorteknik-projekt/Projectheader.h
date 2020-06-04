#include <stdint.h>
//Declare block
const uint8_t const Iblock[4][4][4];
const uint8_t const Jblock [4][4][4];
const uint8_t const Lblock [4][4][4];
const uint8_t const Tblock [4][4][4];
const uint8_t const Oblock [4][4][4];
const uint8_t const Zblock [4][4][4];
const uint8_t const Sblock [4][4][4];

//Global variables
int scoreboard [3];
uint8_t gamefield [20][10];						
int xcoordinate;		
int ycoordinate;	
uint8_t currentblock [4][4][4];
int currentrotation;
int score;
			
int scoreboard [3];
char nameboard [3][3];
char name[3];

//Declare buttons
int button_left ();
int button_right();
int button_rotateleft();
int button_rotateright ();
int8_t screenSelect;

//Declare functions for gamelogic
void displayMenu(void);
void clearScreen();
void num32asc( char * s, int n );
void flashForTETRIS(int number);
int ispossible(int x, int y, int rot);
void MoveRorL ();
void rotation(int rotationvalue);
void updategamefield(int oldx, int oldy, int oldRot);
int fallingblock(int directiondown);
void updatepoints(int rows);
void createnextblock();
void setcurrentblock();
int checkrow(int y);
void ifGAMEOVER();
void highscore (int scores, char *name);
void initiategame();

//Declare memfunctions
#define MEM_ADDRESS 0x50
uint8_t ReadToMem();
void WriteToMem();

//Declare projinit
void projinit();

//Declare user_isr
void user_isr(void);

void enable_interrupt(void);

/* Declare display-related functions from mipslabfunc.c */
void display_image(int x, const uint8_t *data);
void display_init(void);
void display_string(int line, char *s);
void display_update(int8_t x, int8_t y, int8_t roterat);
uint8_t spi_send_recv(uint8_t data);

/* Declare lab-related functions from mipslabfunc.c */
char * itoaconv( int num );

void quicksleep(int cyc);

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare bitmap array containing icon */
extern const uint8_t const icon[128];
/* Declare text buffer for display output */
extern char textbuffer[4][16];
