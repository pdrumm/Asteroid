// Patrick Drumm and Kristopher Thieneman
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "gfx3.h"
#include <string.h>

typedef struct Asteroid{
	float dxAst;
	float dyAst;
	float xAst;
	float yAst;
	int rAst;
	int value;
	int size;
	struct Asteroid *next;
}type_asteroid;

typedef struct Bullet{
	float dxBul;
	float dyBul;
	float xBul;
	float yBul;
	int distance;
	struct Bullet *next;
}type_bullet;

void startGame(int, int);
type_asteroid * makeAsteroid(int,int,int);
void drawAsteroid(type_asteroid *);
void drawRocket(float,float,float,int,int,int);
void wormhole(float *,float *,type_asteroid *,type_bullet *,int,int);
void scoreboard(int);
char *num2str(int);
type_bullet * makeBullet(float, float, float, int, type_bullet *);
void drawBullets(type_bullet *, type_bullet **);
int contactCheck(type_bullet *, type_bullet **, type_bullet *, type_bullet **, type_asteroid *, type_asteroid **, type_asteroid *, type_asteroid **, float *, float *, int, int, double *, float *, float*, int *, int *);
void drawLives(int,int);
void deleteAsteroid(type_asteroid **, type_asteroid **, type_asteroid *, type_asteroid *, type_asteroid *);
void deleteBullet(type_bullet **, type_bullet **, type_bullet *, type_bullet *,  type_bullet *);
void splitAsteroid(type_asteroid **, type_asteroid**, type_asteroid *, type_asteroid *, type_asteroid *);
int pause(int, int);
int gameover(int, int, int);

int main()
{
	// Define variables
	int xSize=800, ySize=600;
	char c;
	int stillgoing=1;
	float xc=xSize/2, yc=ySize/2, *pxc=&xc, *pyc=&yc; // center of square of rocket
	float dr=.2;			// rotation increment
	int r = 15;			// box radius
	double rotatea=0;		// rotation angle of rocket
	float dxRoc=0, dyRoc=0;		// change in position of rocket
	float dt=0;			// change in time for acceleration
	float accel=.17;		// acceleration of rocket
	srand(time(NULL));
	int isaccel=0;
	double rotatet=rotatea;		// rotatet is angle that rocket nose is facing
	int level=0;
	type_asteroid *headAst=NULL, *tailAst, *pAst;
	type_bullet *headBul=NULL, *tailBul=NULL, *pBul;
	int lives = 3;
	int extralives = 1;		// used to know when a player reaches a score that is a multiple of ten thousand
	int score = 0;
	int invincible=1;		// used to make the player invincible for a short period after they lose a life

	gfx_open(xSize,ySize,"Asteroid!");
	
	startGame(xSize, ySize);
	while(stillgoing==1)
	{
		if(headAst==NULL){
			level+=1;
			headAst=makeAsteroid(level,xSize,ySize);
			pAst=headAst;
			while(pAst->next != NULL) pAst=pAst->next;
			tailAst=pAst;
			xc=xSize/2;
			yc=ySize/2;
			dxRoc=0;
			dyRoc=0;
		}

		gfx_clear();
		drawLives(lives,xSize);
		drawAsteroid(headAst);	
		drawRocket(xc,yc,rotatet,r,isaccel,invincible);	// uses rotatet, because this will draw the rocket as turning when the left/right arrows are pressed
		if(headBul!=NULL) drawBullets(headBul, &headBul);

		scoreboard(score);
		gfx_flush();
		usleep(2000);

		isaccel=0;
		if(invincible<=1001) invincible++;

		if(gfx_event_waiting()){
			c=gfx_wait();
			switch(c){
				case 'R':	// Accelerate
					dt+=.01;
					dxRoc+=accel*dt;	// increases speed dx by increasing the time and adding to dx
					if(dxRoc>.6) dxRoc=.6;	// this caps dx and dy at 1. Thus the maximum speed is dx=dy=1
					dyRoc=dxRoc;
					isaccel=1;	// the rocket is currently accelerating
					break;
				case 'Q':	// Turn left
					rotatet-=dr;	// decreases angle of rotation
					break;
				case 'S':	// Turn right
					rotatet+=dr;	// increases angle of rotation
					break;
				case 'T':	// Brake
					dxRoc=0;		// sets speed dx and dy to 0
					dyRoc=0;
					rotatea=rotatet;
					break;
				case ' ':
					tailBul = makeBullet(xc, yc, rotatet, r, tailBul);
					if (headBul == NULL){// meaning bullet created is only one on screen at the moment
						headBul = tailBul;
					}
					break;
				case 'q':	// Quit
					stillgoing = pause(xSize, ySize);
					break;
				default:
					break;
			}
		}
		if(isaccel==1){
			rotatea=rotatet;
		}

		xc+=dxRoc*cos(rotatea);	// implements x and y speed(dx and dy) by changing position(xc,yc) of rocket at varying rate
		yc+=dyRoc*sin(rotatea);	// the rocket will only update the angle of movement, if the rocket is accelerating. This way, you can turn the rocket while it is moving in another direction
		for(pAst=headAst;pAst!=NULL;pAst=pAst->next){// update postion of the asteroids
			pAst->xAst+=pAst->dxAst;
			pAst->yAst+=pAst->dyAst;
		}
		pAst=headAst; // resets pointer to asteroids to the head pointer
		// Transports rocket and asteroids from one side of screen to other
		wormhole(pxc,pyc,headAst,headBul,xSize,ySize);
		lives = contactCheck(headBul, &headBul, tailBul, &tailBul, headAst, &headAst, tailAst, &tailAst, &xc, &yc, r, lives, &rotatet, &dxRoc, &dyRoc, &score, &invincible);
		if (lives <= 0) stillgoing = 0; // ran out of lives, gameover
		if ((score/(10000*extralives)) >= 1){ //extralives starts out at 1 so when player reaches 10000 it adds a life
			lives++;
			extralives++; // increments by 1 so that it will then only add a life when score reaches 20000, etc.
		}

		if (stillgoing == 0){
			gfx_clear();
			stillgoing = gameover(score, xSize, ySize);
			while(headAst!=NULL){
				deleteAsteroid(&headAst,&tailAst,headAst,tailAst,tailAst);
			}
			lives = 3;
			extralives = 1;
			score = 0;
			level = 0;
			invincible=1;
		}
	}
}

void startGame(int xSize, int ySize)
{
	char c;
	char *line1 = "Welcome to Asteroid! Press 'p' to play!";
	char *line2 = "Instructions:";
	char *line3 = "Press space to shoot the asteroids.";
	char *line4 = "Use arrow keys to navigate the ship.";
	char *line5 = "Up to accelrate.";
	char *line6 = "Down to brake immediately.";
	char *line7 = "Left and right to turn the ship.";
	char *line8 = "Press 'q' to pause the game";
	int i = 15; // increment y position by i so the lines do not overlap

	gfx_text((xSize/2)-(strlen(line1)/2)*5, ySize/2-i*3, line1);
	gfx_text((xSize/2)-(strlen(line2)/2)*5, ySize/2-i*2, line2);
	gfx_text((xSize/2)-(strlen(line3)/2)*5, ySize/2-i, line3);
	gfx_text((xSize/2)-(strlen(line4)/2)*5, ySize/2, line4);
	gfx_text((xSize/2)-(strlen(line5)/2)*5, ySize/2+i, line5);
	gfx_text((xSize/2)-(strlen(line6)/2)*5, ySize/2+i*2, line6);
	gfx_text((xSize/2)-(strlen(line7)/2)*5, ySize/2+i*3, line7);
	gfx_text((xSize/2)-(strlen(line8)/2)*5, ySize/2+i*4, line8);

	while (c != 'p') c = gfx_wait();
}

void drawRocket(float xc, float yc, float rotatet, int r, int isaccel, int invincible)
{
	int i, j;
	// Draw Box
	gfx_color(255,255,255);
	if(invincible<=1000){
		if(invincible%100>=50) gfx_color(0,0,0);
		if(invincible%100<50) gfx_color(255,255,255);
	}
	for(i=1;i<=7;i+=2){
		gfx_line(xc+r*cos(rotatet+(i*M_PI)/4), yc+r*sin(rotatet+(i*M_PI)/4), xc+r*cos(rotatet+((i+2)*M_PI)/4), yc+r*sin(rotatet+((i+2)*M_PI)/4));
	}
	// Draw Triangle
	for(j=-1;j<=1;j+=2){
		gfx_line(xc+r*cos(rotatet+(j*M_PI)/4), yc+r*sin(rotatet+(j*M_PI)/4), xc+1.6*r*cos(rotatet), yc+1.6*r*sin(rotatet));
	}
	gfx_color(255,255,255);
	// Draw Flame
	if(isaccel){
		gfx_triangle( xc+r*cos(rotatet+(3*M_PI)/4),yc+r*sin(rotatet+(3*M_PI)/4),xc+2.5*r*cos(rotatet+M_PI),yc+2.5*r*sin(rotatet+M_PI),xc+r*cos(rotatet+(5*M_PI)/4),yc+r*sin(rotatet+(5*M_PI)/4) );
	}
}

void wormhole(float *pxc, float *pyc, type_asteroid *head, type_bullet *headBul, int xSize, int ySize)
{
		type_asteroid *p;
		type_bullet *pBul;
		// Transports rocket from one side of screen to other
		if(*pxc>xSize) *pxc=0;
		if(*pxc<0) *pxc=xSize;
		if(*pyc>ySize) *pyc=0;
		if(*pyc<0) *pyc=ySize;
		// Transports circle from one side of screen to other
		for(p=head;p!=NULL;p=p->next){
			if(p->xAst>xSize) p->xAst=0;
			if(p->xAst<0) p->xAst=xSize;
			if(p->yAst>ySize) p->yAst=0;
			if(p->yAst<0) p->yAst=ySize;
		}
		// Transports bullet from one side of screen to other
		for(pBul=headBul;pBul!=NULL;pBul=pBul->next){
			if(pBul->xBul>xSize) pBul->xBul=0;
			if(pBul->xBul<0) pBul->xBul=xSize;
			if(pBul->yBul>ySize) pBul->yBul=0;
			if(pBul->yBul<0) pBul->yBul=ySize;
		}
}

void drawAsteroid(type_asteroid *head)
{
	type_asteroid *p=head;
	while(p!=NULL){
		gfx_circle(p->xAst,p->yAst,p->rAst);
		p=p->next;
	}
}

void scoreboard(int score)
{
	gfx_text(20, 20, num2str(score));
}

char *num2str(int n)
{
	static char a[10];
	sprintf(a, "%d", n);
	return (char *)a;
}

type_asteroid * makeAsteroid(int level, int xSize, int ySize)
{
	int i;
	double theta;
	type_asteroid *head=malloc(sizeof(type_asteroid));
	type_asteroid *p=head;

	for(i=1;i<=(level+2);i++){
		do{
			p->xAst=rand()%xSize;
			p->yAst=rand()%ySize;
		}while(sqrt(pow(p->xAst-xSize/2,2)+pow(p->yAst-ySize/2,2)) < 150); // create random asteroid position until it is at least a distance of 150 away from the rocket
		p->dxAst=.1*cos(rand());
		p->dyAst=.1*sin(rand());
		p->rAst=40;
		p->value = 20;
		p->size = 0;
		p->next=malloc(sizeof(type_asteroid));
		if(i==level+2) p->next=NULL;
		p=p->next;
	}

	return(head);
}

type_bullet * makeBullet(float xcRoc, float ycRoc, float rotatet, int r, type_bullet *tailBul)
{
	if(tailBul == NULL){
		tailBul = malloc(sizeof(type_bullet));
	}
	else{
		tailBul->next = malloc(sizeof(type_bullet));
		tailBul = tailBul->next;
	}

	tailBul->xBul = xcRoc+1.6*r*cos(rotatet);
	tailBul->yBul = ycRoc+1.6*r*sin(rotatet);
	tailBul->dxBul = .8*cos(rotatet);
	tailBul->dyBul = .8*sin(rotatet);
	tailBul->distance = 0;

	tailBul->next = NULL;
	
	return tailBul;
}

void drawBullets(type_bullet * head, type_bullet **phead)
{
	type_bullet *p = head, *prev = head;
	if(head->distance > 500){
		*phead=p->next;
		free(p);
		p=head;
	}

	while(p != NULL){
		gfx_text(p->xBul, p->yBul, ".");
		p->xBul += p->dxBul;
		p->yBul += p->dyBul;
		p->distance += 1;
		prev = p;
		p = p->next;
	}
}

int contactCheck(type_bullet *headBul, type_bullet **pheadBul, type_bullet *tailBul, type_bullet **ptailBul, type_asteroid *headAst, type_asteroid **pheadAst, type_asteroid *tailAst, type_asteroid **ptailAst, float *xc, float *yc, int r, int lives, double *rotatet, float *dxRoc, float *dyRoc, int *score, int *invincible)
{
	type_asteroid *pAst;
	type_bullet *pBul;

	//check if rocket hits asteroid
	if(*invincible>=1000){
		for(pAst=headAst;pAst!=NULL;pAst=pAst->next){
			if(sqrt(pow((*xc-pAst->xAst),2)+pow((*yc-pAst->yAst),2))<r+pAst->rAst || sqrt(pow((*xc+1.6*r*cos(*rotatet))-pAst->xAst,2) + pow((*yc+1.6*r*sin(*rotatet))-pAst->yAst,2))<pAst->rAst){
				lives--;
				*xc = 400;
				*yc = 300;
				*dxRoc = 0;
				*dyRoc = 0;
				*rotatet = 0;
				*score += pAst->value;
				*invincible=1;
				splitAsteroid(pheadAst, ptailAst, pAst, tailAst, headAst);
			}
		}
	}

	//check if bullet hits asteroid
	for(pBul=headBul; pBul != NULL; pBul = pBul->next){
		for(pAst=headAst;pAst!=NULL;pAst=pAst->next){
			if (sqrt(pow((pBul->xBul-pAst->xAst),2)+pow((pBul->yBul - pAst->yAst),2))<pAst->rAst){
				deleteBullet(pheadBul,ptailBul,headBul,tailBul,pBul);
				*score += pAst->value;
				splitAsteroid(pheadAst, ptailAst, pAst, tailAst, headAst);
			}
		}
	}

	return lives;
}

void drawLives(int lives, int xSize)
{
	gfx_color(255,255,255);
	int xc = xSize-40, yc = 20, width=10, i;
	for(i=1;i<=lives;i++){
		gfx_rectangle(xc,yc,width,width);
		gfx_triangle(xc,yc,xc+width,yc,xc+width*.5,yc-width*.6);
		xc-=30;
	}
}

void deleteAsteroid(type_asteroid **phead, type_asteroid **ptail, type_asteroid *head, type_asteroid *tail, type_asteroid *p)
{
	type_asteroid *prev, *search;
	if(p==head){
		prev=head;
		*phead=head->next;
	}else{// p or tail
		for(search=head;search!=p;search=search->next)
			prev=search;
		if(search==tail) *ptail=prev;
		prev->next=p->next;
	}
}

void deleteBullet(type_bullet **phead, type_bullet **ptail, type_bullet *head, type_bullet *tail,  type_bullet *p)
{
	type_bullet *prev, *search;
	if(p==head){
		prev=head;
		*phead=head->next;
	}else{// p or tail
		for(search=head;search!=p;search=search->next)
			prev=search;
		if(search==tail) *ptail=prev;
		prev->next=p->next;
	}
}

void splitAsteroid(type_asteroid **pheadAst, type_asteroid **ptailAst, type_asteroid *pAst, type_asteroid *tailAst, type_asteroid *headAst)
{
	if (pAst->rAst >= 18){
		pAst->size += 1;
		pAst->rAst /= 1.5;
		pAst->dxAst = pow(1.5, pAst->size)*.1*cos(rand());
		pAst->dyAst = pow(1.5, pAst->size)*.1*sin(rand());
		if(pAst->value == 20) pAst->value = 50;
		else if(pAst->value == 50) pAst->value = 100;

		tailAst->next = malloc(sizeof(type_asteroid));
		tailAst=tailAst->next;
		tailAst->next=NULL;
		tailAst->size = pAst->size;
		tailAst->xAst = pAst->xAst;
		tailAst->yAst = pAst->yAst;
		tailAst->dxAst = pow(1.5, tailAst->size)*.1*cos(rand());
		tailAst->dyAst = pow(1.5, tailAst->size)*.1*sin(rand());
		tailAst->rAst = pAst->rAst;
		tailAst->value = pAst->value;
		*ptailAst = tailAst;
	}else{
		deleteAsteroid(pheadAst, ptailAst, headAst, tailAst, pAst);
	}

}

int pause(int xSize, int ySize)
{
	char c;
	char *pauseMenu = "Quit Game? Y/N";
	int i = 15;

	gfx_text(xSize/2 - strlen(pauseMenu)/2*5, ySize/2+i, pauseMenu);

	while(c != 'y' && c != 'n') c = gfx_wait();
	
	if(c == 'n') return 1;
	if(c == 'y') return 0;
}

int gameover(int score, int xSize, int ySize)
{
	char c;
	char *gameover = "Gameover";
	char *lineScore = "Your score is :";
	char *finalScore = num2str(score);
	char *playAgain = "Would you like to play again? Y/N";
	int i = 15;

	gfx_text((xSize/2) - (strlen(gameover)/2)*5, ySize/2, gameover);
	gfx_text((xSize/2) - (strlen(lineScore)/2)*5, ySize/2+i, lineScore);
	gfx_text((xSize/2) - (strlen(finalScore)/2)*5, ySize/2+2*i, finalScore);
	gfx_text((xSize/2) - (strlen(playAgain)/2)*5, ySize/2+3*i, playAgain);

	while(c != 'y' && c != 'n') c = gfx_wait();

	if(c == 'y') return 1;
	if(c == 'n') return 0;
}
