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

void Bounce(int button){

    while(waitForInterrupt(button, 100) || (digitalRead(button) == HIGH)){
        
        }
}

void Button1Pressed(){
    Bounce(BUTTON_1);    

    printf("Button 1 pressed..\n");    
    buttonHandle(1);
}

void Button2Pressed(){
    Bounce(BUTTON_2);       

    printf("Button 2 pressed..\n");        
    buttonHandle(2);       
}

void Button3Pressed(){
    Bounce(BUTTON_3);

    printf("Button 3 pressed..\n");        
    buttonHandle(3); 
}


int main(){

    printf("Application started!\n");

    sethandler(shutdown_handler, SIGINT);

    if(wiringPiSetupGpio () < 0)
        ERR("WiringPi cannot be initialized\n");

    initializePins();

    if(wiringPiISR(BUTTON_1, INT_EDGE_BOTH, Button1Pressed) < 0)
        ERR("wiringPiISR error for button 1\n");
    if(wiringPiISR(BUTTON_2, INT_EDGE_BOTH, Button2Pressed) < 0)
        ERR("wiringPiISR error for button 2\n");
    if(wiringPiISR(BUTTON_3, INT_EDGE_BOTH, Button3Pressed) < 0)
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