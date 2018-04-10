#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "gpioMapping.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__), perror(source),kill(0,SIGKILL), exit(EXIT_FAILURE))

int isRunning = 1;

void sethandler( void (*f)(int), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void shutdown_handler(int sigNo){
    printf("\n");
    isRunning = 0;       
}

void initializePins(){
    pinMode (LED_RED, OUTPUT);
    pinMode (LED_GREEN, OUTPUT);
    pinMode (LED_WHITE, OUTPUT);
    pinMode (LED_BLUE, OUTPUT);

    pinMode (BUTTON_1, INPUT);
    pinMode (BUTTON_2, INPUT);
    pinMode (BUTTON_3, INPUT);    
}

int BUTTONS_STATE[3];


void Button1Pressed();
void Button2Pressed();
void Button3Pressed();

void Button1Released(){
    piLock(0);     
    if(BUTTONS_STATE[0] == 0)
    {
        piUnlock(0);   
        return;
    }

    printf("Button1 Released\n");    
    BUTTONS_STATE[0] = 0;
    piUnlock(0);    
    
    if(wiringPiISR(BUTTON_1, INT_EDGE_RISING, Button1Pressed) < 0)
        ERR("wiringPiISR error for button 1\n");

}

void Button2Released(){
     piLock(1);     
    if(BUTTONS_STATE[1] == 0)
    {
        piUnlock(1);   
        return;
    }
    printf("Button2 Released\n");  

    BUTTONS_STATE[1] = 0;
    piUnlock(1);   
    if(wiringPiISR(BUTTON_2, INT_EDGE_RISING, Button2Pressed) < 0)
        ERR("wiringPiISR error for button 2\n");
}

void Button3Released(){
    piLock(2);     
    if(BUTTONS_STATE[2] == 0)
    {
        piUnlock(2);   
        return;
    }
    printf("Button3 Released\n");   

    BUTTONS_STATE[2] = 0;
    piUnlock(2); 

    if(wiringPiISR(BUTTON_3, INT_EDGE_RISING, Button3Pressed) < 0)
        ERR("wiringPiISR error for button 3\n");

}

void Button1Pressed(){
    piLock(0);        
    if(BUTTONS_STATE[0] == 1)
    {
        piUnlock(0);                 
        return;
    }

    printf("Button1 Pressed\n");
    BUTTONS_STATE[0] = 1;
    piUnlock(0);          
    
    if(wiringPiISR(BUTTON_1, INT_EDGE_FALLING, Button1Released) < 0)
        ERR("wiringPiISR error for button 1\n");
}

void Button2Pressed(){
    piLock(1);        
    if(BUTTONS_STATE[1] == 1)
    {
        piUnlock(1);                 
        return;
    }

    printf("Button2 Pressed\n");
    BUTTONS_STATE[1] = 1;
    piUnlock(1);         

    if(wiringPiISR(BUTTON_2, INT_EDGE_FALLING, Button2Released) < 0)
        ERR("wiringPiISR error for button 2\n");
}

void Button3Pressed(){

    piLock(2);        
    if(BUTTONS_STATE[2] == 1)
    {
        piUnlock(2);                 
        return;
    }

    printf("Button3 Pressed\n");
    BUTTONS_STATE[2] = 1;
    piUnlock(2);
  
    if(wiringPiISR(BUTTON_3, INT_EDGE_FALLING, Button3Released) < 0)
        ERR("wiringPiISR error for button 3\n");
}


int main(){

    printf("Application started!\n");
    sethandler(shutdown_handler, SIGINT);

    if(wiringPiSetupGpio () < 0)
        ERR("WiringPi cannot be initialized\n");

    if(wiringPiISR(BUTTON_1, INT_EDGE_RISING, Button1Pressed) < 0)
        ERR("wiringPiISR error for button 1\n");
    if(wiringPiISR(BUTTON_2, INT_EDGE_RISING, Button2Pressed) < 0)
        ERR("wiringPiISR error for button 2\n");
    if(wiringPiISR(BUTTON_3, INT_EDGE_RISING, Button3Pressed) < 0)
        ERR("wiringPiISR error for button 3\n");
    
    
    while(isRunning)
    {
        
    }

    printf("Application shutdown!\n");    
    return 0;
}