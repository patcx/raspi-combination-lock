#include <wiringPi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "gpioMapping.h"
#include "states.h"

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__), perror(source),kill(0,SIGKILL), exit(EXIT_FAILURE))
#define PASSWORD_LENGTH 4

int ERRORS = 0;
int IS_RUNNING = 1;
int PASSWORD_POSITION;
int SET_PASSWORD[PASSWORD_LENGTH];
int PASSWORD[PASSWORD_LENGTH];
int BUTTONS_STATE[3];
int STATE = UNLOCKED;
pthread_cond_t CONDITION = PTHREAD_COND_INITIALIZER;;
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;;

void sethandler( void (*f)(int), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void shutdown_handler(int sigNo){
    printf("\n");
    pthread_cond_signal(&CONDITION);    
}

void initializePins(){
    pinMode (LED_RED, OUTPUT);
    pinMode (LED_GREEN, OUTPUT);
    pinMode (LED_WHITE, OUTPUT);
    pinMode (LED_BLUE, OUTPUT);

    digitalWrite (LED_RED, LOW);
    digitalWrite (LED_GREEN, LOW);
    digitalWrite (LED_WHITE, LOW);
    digitalWrite (LED_BLUE, LOW);
    

    pinMode (BUTTON_1, INPUT);
    pinMode (BUTTON_2, INPUT);
    pinMode (BUTTON_3, INPUT);    
}

void alarm(){
    int i;
    for(i=0; i<3; i++){
        if(ERRORS<3){
            digitalWrite(LED_RED, HIGH);
            delay(300);
            digitalWrite(LED_RED, LOW);     
            delay(300);  
        }         
        else{
            digitalWrite(LED_RED, HIGH);
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_WHITE, HIGH);
            digitalWrite(LED_BLUE, HIGH);
            delay(300);
            digitalWrite(LED_RED, LOW);     
            digitalWrite(LED_GREEN, LOW);     
            digitalWrite(LED_WHITE, LOW);     
            digitalWrite(LED_BLUE, LOW);     
            delay(300);  
        }
    }
}

void update(){

    if(PASSWORD_POSITION >= PASSWORD_LENGTH){
        if(STATE == UNLOCKED){
            int i;
            for(i=0; i<PASSWORD_LENGTH; i++){
                SET_PASSWORD[i] = PASSWORD[i];
            }
            STATE = LOCKED;
        }
        else if(STATE == LOCKED){
            int i;
            int unlocked = 1;
            for(i=0; i<PASSWORD_LENGTH; i++){
                if(SET_PASSWORD[i] != PASSWORD[i])
                    unlocked = 0;
            }
            if(unlocked)
            {
                STATE = UNLOCKED;
                ERRORS = 0;
            }
            else{
                ERRORS++;
                alarm();                
            }
            
        }   

        PASSWORD_POSITION = 0;
    }
    
    switch(STATE){
        case UNLOCKED:
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_RED, LOW);
            break;
        case LOCKED:
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_RED, HIGH);
            break;
    }

}

void buttonHandle(int number){

    PASSWORD[PASSWORD_POSITION++] = number;

    digitalWrite(LED_WHITE, HIGH);
    delay(300);
    digitalWrite(LED_WHITE, LOW);     
    delay(300);    

    update();
}


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

    buttonHandle(1);
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
    buttonHandle(2);
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
    buttonHandle(3);
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

    initializePins();

    if(wiringPiISR(BUTTON_1, INT_EDGE_RISING, Button1Pressed) < 0)
        ERR("wiringPiISR error for button 1\n");
    if(wiringPiISR(BUTTON_2, INT_EDGE_RISING, Button2Pressed) < 0)
        ERR("wiringPiISR error for button 2\n");
    if(wiringPiISR(BUTTON_3, INT_EDGE_RISING, Button3Pressed) < 0)
        ERR("wiringPiISR error for button 3\n");
    
    update();
    pthread_cond_wait(&CONDITION, &MUTEX);

    digitalWrite (LED_RED, LOW);
    digitalWrite (LED_GREEN, LOW);
    digitalWrite (LED_WHITE, LOW);
    digitalWrite (LED_BLUE, LOW);
    
    printf("Application shutdown!\n");    
    return 0;
}