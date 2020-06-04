#include <stdint.h>   
#include <pic32mx.h> 
#include "Projectheader.h"

/*
Written 2020 by Frida Johansson
ClearBlock and makeBlockPermanent written by David Strömqvist
Added: FlashforTetris in user_isr by David Strömqvist
*/

#define CLOCKTICKPERSECOND 50

int gameSpeedTicks = CLOCKTICKPERSECOND - 10;
int gamecounter = 0;									//counter för fallhastighet
int score = 0;											
int movecounter = 0;									//counter för rörelse till höger eller vänster
int rotationcounter = 0;								//counter för rotation av block
uint8_t gamefield [20][10];								//Spelplan
uint8_t currentblock [4][4][4];							//Nuvarande block
int xcoordinate = 0;									//Nuvarande xkoordinat
int ycoordinate = 0;									//Nuvarande ykoordinat
int currentrotation = 0;								//Nuvarande rotation
uint8_t nextblock [4][4][4];							//Nästa block
int nextrotation = 0;									//Nästa rotation
int flashDelay = 0;

int gameSpeed = 0;
int removedRows = 0;
int fallDownCounter = 0;
int flashForTETRIScounter = 0;

void clearBlock();
void removeFullRows();


/*
Functions: button_left, button_right,
button_rotateleft, button_rotateright
Returns the int-value
*/
int button_left (){
	 return (PORTD >> 6) & 0x1;
}
int button_right(){
	return (PORTD >> 5) & 0x1;
}
int button_rotateleft(){
	return (PORTD >> 7) & 0x1;
}
int button_rotateright (){
	return (PORTF >> 1) & 0x1;
}
int button_fall_down()
{
	return (PORTD >> 8) & 0x3;
}

/*
Function Initiategame
Go through the gamefield, clears the screen 
and initiate a new game
*/
void initiategame(){
	int i,j;
	for(i = 0; i < 20; i++){
		for(j = 0; j < 10; j++){
			gamefield[i][j] = 0; 
		}
	}
	clearScreen();
	createnextblock();
	setcurrentblock();
	xcoordinate = 3;
	score = 0;

}
/*
Function makeBlockPermanent
Block keeps shape during rotation 
*/
void makeBlockPermanent()
{
	int i, j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(currentblock[currentrotation][i][j] != 0){
				gamefield[ycoordinate + i][xcoordinate + j] = 2;
			}
		}
	}
}

/*
Function clearBlock
Clears block
*/
void clearBlock()
{
	int i,j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(currentblock[currentrotation][i][j] != 0){
				gamefield[ycoordinate + i][xcoordinate + j] = 0;
			}
		}
	}
}

/*
Function: rotation
rotates current block
*/
void rotation(int rotationvalue){
	int oldRotation = currentrotation;
	int newrotation = rotationvalue + currentrotation;
	clearBlock();
	if(newrotation < 0){
		newrotation = 3;
	}
	else if(newrotation > 3){
		newrotation = 0;
	}
	 
	if(ispossible(xcoordinate, ycoordinate, newrotation)){
		currentrotation = newrotation;
	}
	updategamefield(ycoordinate,xcoordinate,oldRotation);
}

/*
Function: ispossible
checks if new location is possible
*/
int ispossible(int x, int y, int rot){
	int i, j;
	for(i = 0; i < 4 ; i ++){
		for(j = 0; j < 4; j++){
			if(currentblock[rot][i][j] != 0){				//j = xkoordinat, i = ykoordinat, rot = olika rotationer
				if((j + x < 0) || (j + x >= 10)){
					return 0;
				}
				if((gamefield[i+y][j+x] == 2) || (y + i >= 20)){
					 return 0;
				 }
			}				
		}
	}
	return 1;
}
/*
Function: MoveRorL
Moves block left or right
*/
void MoveRorL (int direction){
	int oldx = xcoordinate;
	if(ispossible(xcoordinate + direction, ycoordinate, currentrotation)){
		xcoordinate = xcoordinate + direction;
	}
	
	updategamefield(oldx, ycoordinate, currentrotation);
}

/*
Function: fallingblock
function that makes the block fall down one step on y-axis
*/
int fallingblock(int directiondown){
	int oldy = ycoordinate;
	if(ispossible(xcoordinate, ycoordinate + directiondown, currentrotation)){
		ycoordinate = ycoordinate + directiondown;
		updategamefield(xcoordinate, oldy, currentrotation);
	}
	else{
		
		makeBlockPermanent();
		removeFullRows();
		setcurrentblock();
		ifGAMEOVER();
	}
	
}
void setcurrentblock(){
	int i, j, k;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			for(k = 0; k < 4; k++){
				currentblock[i][j][k] = nextblock[i][j][k];
			}
		}
	}
	currentrotation = nextrotation; 
	ycoordinate = 0;
	
	int flag = 0;
	for(j = 0; j < 4; j++){
		if(currentblock[currentrotation][0][j] != 0){
			flag = 1;
			break;
		}		
	}
	if(!flag)
		ycoordinate--;
	xcoordinate = 3;
	createnextblock();
	
}

/*
Function createnextblock
creates a random block
*/
void createnextblock(){
	int randomnumber = rand() % 7;
	int i,j,k;
	switch(randomnumber){
		case 0:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Iblock[i][j][k];
					}
				}
			}
			break;
		case 1:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Jblock[i][j][k];
					}
				}
			}
			break;
		case 2:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Lblock[i][j][k];
					}
				}
			}
			break;
		case 3:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Tblock[i][j][k];
					}
				}
			}
			break;
		case 4:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Oblock[i][j][k];
					}
				}
			}
			break;
		case 5:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Zblock[i][j][k];
					}
				}
			}
			break;
		case 6:
			for(i = 0; i < 4; i++){
				for(j = 0; j < 4; j++){
					for(k = 0; k < 4; k++){
						nextblock[i][j][k] = Sblock[i][j][k];
					}
				}
			}
			break;
		
	}
	randomnumber = rand() % 4;
	nextrotation = randomnumber;		
}


/*
Function: updategamefield
Updates the gamefield
*/
void updategamefield(int oldx, int oldy, int oldRot){
	int i,j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(currentblock[oldRot][i][j] != 0){
				gamefield[oldy + i][oldx + j] = 0;
			}
		}
	}
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(currentblock[currentrotation][i][j] != 0){
				gamefield[ycoordinate + i][xcoordinate + j] = 1;
			}
		}
	}
}
/*
Function removeFullRows
remove row when filled
*/	
void removeFullRows()
{
	int i, j, y;
	int zerorow = 0;
	int currentrow = 0;
	int rowforpoints = 0;
	for(i = 0; i < 20 ; i++){
		currentrow = checkrow(i);
		if(currentrow == 0)
			zerorow = i;
		else if(currentrow == 10){
			rowforpoints ++;
			for(y = i; y > zerorow; y--){
				for(j = 0; j < 10; j++){
					gamefield[y][j] = gamefield[y - 1][j];
				}
			}
		}
	}
	removedRows += rowforpoints;
	if( (gameSpeed <= 10 )&& (removedRows > 20) )
	{
		gameSpeed++;
		removedRows = 0;
		gameSpeedTicks = CLOCKTICKPERSECOND - (gameSpeed * 2);
	}
	if(rowforpoints > 0)
		updatepoints(rowforpoints);
}
/*
Function: updatepoints
Updates the player's score
*/
void updatepoints(int rows){
	switch(rows){
		case 1:
			score += 10;
			break;
		case 2:
			score += 25;
			break;
		case 3:
			score += 45;
			break;
		case 4:
			score += 75;
			flashForTETRIScounter = 4;
			break;
	}
}

/*
Function: checkrow
return 10 if the row is full
return 0 if the row is empty
return 0<int<=9 if the row is not completely filled
*/
int checkrow(int y){
	int value = 0;
	int j;
	for(j = 0; j < 10; j++){
		if(gamefield[y][j] == 2)
			value ++;	
	}
	return value;
}
/*
Function: ifGAMEOVER
Ends the game when no blocks fits the gamefield
*/
void ifGAMEOVER(){
	if(ispossible(xcoordinate,ycoordinate,currentrotation) == 0){
		screenSelect = 3;
		display_string(0, " GAME OVER");
		display_string(1, " You lost!");
		display_string(2, " score ");
		display_string(3, itoaconv(score));
		displayMenu();
	}
}


/*
Function: projinit
Initializing random, timer2, buttons, interrupts
*/
void projinit(){
	
	
	//Timer2
	T2CON = 0x0;											//Stannar timers och clears timer 
	T2CONSET = 0x70;										//Prescale 1:256										
	TMR2 = 0;												//Sätter räknaren till 0.
	PR2 = 6250; 											//80MHz/256/50, 16 bits
	
	T2CONSET =  0x8000;										//Startar klockar
	
	//Buttons
	TRISDSET = 0x1f << 5;									//Knapp 2-4
	TRISFSET = 0x1 << 1;									//Knapp 1
	
	//Interrupts
	IECSET(0) = (1 << 8);									//bit 8 till 1.
	IPCSET(2) = (3 << 2);									//Sätt priority
	IFS(0) &= ~0x100;										//Clear bit 8
	
	enable_interrupt();
}

/* Interrupt Service Routine 
 Function: user_isr
*/
void user_isr(void) //todo
{
	if(screenSelect == 0)
	{
		if(flashForTETRIScounter > 0)
		{
			if(flashDelay >= 1)
			{
				flashForTETRIScounter--;
				flashDelay = 0;
				flashForTETRIS(flashForTETRIScounter);
			}else
				flashDelay++;
		}
		
		if (button_fall_down())
		{
			if(fallDownCounter >= 2)
			{
				fallingblock(1);
				fallDownCounter = 0;
			}
		}
		else
		{
			if (gamecounter >= gameSpeedTicks){
				
				fallingblock(1);
				
				gamecounter = 0;
			}
			if (button_left() && movecounter >= 10){
				
				MoveRorL(-1);
				movecounter = 0;
			}
			if(button_right() && movecounter >= 10){
				
				MoveRorL(1);
				movecounter = 0;
			}
			if (button_rotateleft() && rotationcounter >= 10){
				
				rotation(-1);
				rotationcounter = 0;
			}
			if (button_rotateright() && rotationcounter >= 10){
				
				rotation(1);
				rotationcounter = 0;
			}
		}
		display_update(0,0,0);
		fallDownCounter++;
		rotationcounter++;
		movecounter++;
		gamecounter++;
	}
	IFSCLR(0) = 0x100;
    return;
}
