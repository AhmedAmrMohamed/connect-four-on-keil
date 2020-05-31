
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include <stdio.h>
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(unsigned long count); // time delay in 0.1 seconds


// *************************** Images ***************************

// cursor is the only sprite used throught this project
// when it's drawn with a threshold of 0 it's a cross(player 2),
// otherwise (14) it's a square(player 1) 
const unsigned char cursor[] ={
 0x42, 0x4D, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80,
 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF,
 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x4F, 0xF4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFF, 0xFF, 0x4F, 0xF4, 0xFF, 0xFF, 0xFF,

};

// sage22 was only used during early stages of this code, as an abstract.
// it was a friendly 2*2 pixel sprite, and will not be forgotten.
const unsigned char sage22[] ={
 0x42, 0x4D, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x0E, 0x00, 0x00, 0xC3, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x37, 0x52, 0x9B, 0x3A, 0x36, 0x54, 0xAA, 0x3B, 0x67,
 0x92, 0xAC, 0x3F, 0x6E, 0x99, 0xCE, 0xFF,
};
	
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))


/********consts*******/	
const int BOARDWIDTH = 13;
const int BOARDHIGHT = 8;
const int CELLSPACING = 6;

/*******endgamemsgs*****/
char sqwinmsg[] = "SQUARE WINS";
char crwinmsg[] = "CROSS  WINS";
char tiemsg[]      = "TIE!";
char frmsg[]    = "you still friends";

/*******singeltons*********/
int sage     = 0;     // singelton determines which column will be played in.
int turn     = 1;     // 0->first player , 1 -> secondplayer
int gameover = 0;     //0 -> still in play. 1->game over.
int board[BOARDWIDTH][BOARDHIGHT] = {0};  // 2d-array represent the playfield, 0 -> empty
int winner   = 0;

/*****debug singeltons*****/
int debugvalid;
int debugplayed = 0; 

/*******switches hacking******/
unsigned long SW1,SW2;
void PortF_Init(void);
int read_sw1() { return GPIO_PORTF_DATA_R&0x10;}
int read_sw2() { return GPIO_PORTF_DATA_R&0x01;}
int pressedsw2 = 0;

/****************** declared fucntions; gotta love c ********/
void move_sage(void);
void display_board(void);
void display_sage(void);
void display_chipdrop(int,int); 
void move_sage(void);
int put(int);
int toggleturn(int);
int play(void);
int getcell(int,int);
int check_cell(int,int,int);
int check_board(int);
int valid(int,int);

int main(void){	
    TExaS_Init(SSI0_Real_Nokia5110_Scope);
	PortF_Init();
    Nokia5110_Init();
	Nokia5110_ClearBuffer();
    while(!gameover && !winner){
        if(!read_sw1())
	        move_sage();
        
        if(!pressedsw2 && !read_sw2()&& read_sw1()){
                winner = play();      //winner is 0 untill someone wins
                pressedsw2=1;
            }

        if(read_sw2())
            pressedsw2=0;
        if(debugplayed == BOARDHIGHT * BOARDWIDTH)
            break;                     //the board is full, tie.
        display_board();
        display_sage();
        Nokia5110_DisplayBuffer();     // draw buffer
        Delay100ms(1);                 // delay 1 sec at 50 MHz
        Nokia5110_Clear();
        Nokia5110_ClearBuffer();        
    }
    Nokia5110_Clear();
    Nokia5110_ClearBuffer();
    Nokia5110_Clear();
    Nokia5110_SetCursor(1, 1);
    Nokia5110_OutString("GAME OVER");
    Nokia5110_SetCursor(1, 2);
    if(!winner){
       Nokia5110_OutString(tiemsg);
       Nokia5110_OutString(frmsg);
    }
    if(winner ==2 )
        Nokia5110_OutString(crwinmsg);
    if(winner == 1)
        Nokia5110_OutString(sqwinmsg);
    Nokia5110_SetCursor(1, 3);
    Nokia5110_SetCursor(2, 4);
}
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}

void Delay100ms(unsigned long count){unsigned long volatile time;
    while(count>0){
        time = 727240;  // 0.1sec at 80 MHz
        while(time){
	  	    time--;
        }
        count--;
    }
}

void move_sage(void){
    // move sage across the board. called when sw1 is pressed
	sage = (sage+1)%BOARDWIDTH;
}

int put(int turn){
	// try to insert a chip at the top of col pointed to by
	// sage, if the col is full return -1, otherwise return the
	// row where the chip was inserted.
	int row = 0;
    while( !board[sage][row] && row < BOARDHIGHT){
	    display_chipdrop(row,turn);	
        ++row;
    }
	if(!row)  //this column is full, cannot insert chip.
		return -1;
	board[sage][row-1] = turn; // set the cell to 1 or 2
	return row;
}

int play(void){
    // what everytime someone try to make a move (ie: play)
    // triggered everytiem  sw2 is pressed.
    int valid = put(turn+1);
    debugvalid = valid;
    if(valid ==-1)
        return 0;
    if(check_board(turn+1))
        return turn+1;
    turn = toggleturn(turn);
    debugplayed++;
    return 0;
}

int toggleturn(int turn){return (turn+1)%2;} // switch between players 0 and 1
/*********************** Display funcs ****************/ 
void display_board(){
	// display the board on the srcreen.
	int col,row;
	for(col=0;col<BOARDHIGHT;col++){
        for(row=0;row<BOARDWIDTH;row++){
		    int x = board[row][col];
		    if(x==1)
		    	Nokia5110_PrintBMP((row+1) * CELLSPACING, 5*(col+2), cursor , 0); //draw a cross
		    if(x==2)
		    	Nokia5110_PrintBMP((row+1) * CELLSPACING, 5*(col+2), cursor , 14); //draw a rect
		}
	}
}

void display_sage(void){
    //display sage in the correct position according to sageposx
    int posy = 5;
    int posx = (sage+1) * CELLSPACING;
	Nokia5110_PrintBMP(posx,posy, cursor , (turn?14:0));
}
void display_chipdrop(int row,int turn){
    Nokia5110_ClearBuffer();
    board[sage][row] = turn;
    display_board();
    board[sage][row] = 0;
    Nokia5110_DisplayBuffer();
    Delay100ms(1);
    Nokia5110_ClearBuffer();
}

/******** check for win funcs********/
int valid(int A,int B){
    //return 1 if (A,B) is a valid cell(ie: inside the board)
    //0 otherwise.
    return A < BOARDWIDTH && B < BOARDHIGHT;
}

int check_board(int trgt){
    //return 1 if the last move is a winning move, 0 otherwise.
    //triggerd every time a valid move is played.
    int col = 0, row = 0;
	for(col=0;col<BOARDHIGHT;col++){
        for(row=0;row<BOARDWIDTH;row++){
            if(board[row][col] == trgt)
                if(check_cell(row,col,trgt))
                    return 1;
        }
    }
    return 0;
}

int check_cell(int row,int col,int trgt){
    //return 1 if the current cell is the start of
    //a winning sequence
    int okrow = 1;
    int okcol = 1;
    int okbil = 1;
    int okbir = 1;
    int i;
    for(i=1;i<4;i++){
        if(getcell(row+i,col  )!=trgt)
            okrow = 0;
        if(getcell(row  ,col+i)!=trgt)
            okcol = 0;
        if(getcell(row+i,col+i)!=trgt)
            okbil = 0;
        if(getcell(row+i,col-i)!=trgt)
            okbir = 0;
    }
    if(okrow+okcol+okbil+okbir) return 1;
        return 0;
}

int getcell(int row,int col){
    //return the content of a cell, if it is 
    //not on the board return 0
    if(valid(row,col)) return board[row][col];
    else return 0;
}

