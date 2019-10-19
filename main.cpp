#include "C12832.h"
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
BusIn joy (p15,p12,p13,p16,p14);
C12832 lcd(p5,p7,p6,p8,p11);
QueueHandle_t xQueue;
void initialize_led();
int iPosition=0;

enum key_pressed_t {
NONE=0,
LEFT,
RIGHT,
UP,
DOWN,
OK
};

enum fsm_state_t {
TEMPERATURE=1,
QUADRATIC,
RGB
};

void Task1 (void* pvParameters) // sender (toma el teclado como input y lo coloca en la cola. en base a eso se seleccionara la tarea siguiente)
{

    key_pressed_t key_pressed, prev_key_pressed=NONE;
    BaseType_t xStatus;
    
    (void) pvParameters;
    
    for (;;) {
        //if (key_pressed!=prev_key_pressed)
                
        switch (joy)
        {
             case 0:
             //lcd.printf("UP");
             if (key_pressed!=NONE)
            {
                xStatus=xQueueSendToBack(xQueue,(void *)&key_pressed,0);
                if (xStatus!=pdPASS) printf("Could not send to queue. \r\n");
            }
             key_pressed = NONE;             


             break;
    
             case 1:
             //lcd.printf("UP");
             key_pressed = UP;             
             break;
             case 2:
             key_pressed=DOWN;
             
             //lcd.printf("DOWN");
             break;
             case 4:  //left
             key_pressed=LEFT;
             
             break;
             case 8: //right
             key_pressed=RIGHT;
            
             break;
             case 16:
             key_pressed=OK;             
             break;
        //led1 = !led1;
        //printf("Task1\n");
        //vTaskDelay(500);
        }
        printf("Aprete: %d",key_pressed);
        vTaskDelay(30);

    
    }
}
void Task2 (void* pvParameters) //receiver - state machine
{
    key_pressed_t key_pressed;
    fsm_state_t state;
    const TickType_t xCycleFrequency = pdMS_TO_TICKS( 100UL );
    BaseType_t xStatus;


    state = TEMPERATURE;
    
    (void) pvParameters;
    for (;;) {
     lcd.cls();
     lcd.locate(0,15);
     lcd.printf ("STATE: %c",state);
    if (uxQueueMessagesWaiting (xQueue)!=0)
    {
        lcd.printf("Queue should have been empty!\r\n"); 
    }
   xStatus = xQueueReceive (xQueue, (void *)&key_pressed, xCycleFrequency);
   if (xStatus==pdPASS)
   {
    
      switch (state)  // TEMP<->QUAD<->RGB
        {
             case TEMPERATURE:
                 if (key_pressed == LEFT )
                 {  
                     state = RGB;
                 }
                 else if ( key_pressed == RIGHT)
                 {
                     state = QUADRATIC;
                 }
                 else
                 {
                    state = TEMPERATURE;
                    /* code */
                 }
            case QUADRATIC:
                 if (key_pressed == LEFT )
                 {  
                     state = TEMPERATURE;
                 }
                 else if ( key_pressed == RIGHT)
                 {
                     state = RGB;
                 }
                 else
                 {
                    state = QUADRATIC;
                    /* code */
                 }           
             break;
             case RGB:
             if (key_pressed == LEFT )
                 {  
                     state = QUADRATIC;
                 }
                 else if ( key_pressed == RIGHT)
                 {
                     state = TEMPERATURE;
                 }
                 else
                 {
                    state = RGB;
                    /* code */
                 }
             break;  

             default:
                state = TEMPERATURE;
             break;           
        }
      }
     //   led2= !led2;
     //   printf("Task2\n");
     //   vTaskDelay(1000);
    }
}
void Task3 (void* pvParameters) //Menu printer
{
    (void) pvParameters;                    // Just to stop compiler warnings.
    /*int state;
    
    char cMainmenuarray[3][20]={	
	"Temperature",			
	"Quadratic Signal",		
	"RGB",	
    };
    for (;;) {
    
    switch (state)
         {
             case 1:
             //lcd.printf("UP");
             key_pressed=UP;
             break;
             case 2:
             key_pressed=DOWN;
             //lcd.printf("DOWN");
             break;
             case 4:  //left
             key_pressed=LEFT;
               if (iPosition==0) 
               iPosition=sizeof(cMainmenuarray)/sizeof(cMainmenuarray[0])-1; //5limite der, reset scroll
               else iPosition--;
               lcd.cls();
               lcd.printf(cMainmenuarray[iPosition]);
               lcd.locate(0,15);
             break;
             case 8: //right
             key_pressed=RIGHT;
               if (iPosition==sizeof(cMainmenuarray)/sizeof(cMainmenuarray[0])-1) 
               iPosition=0; //limite izq, reset scroll
               else iPosition++; 
               lcd.cls();           
               lcd.printf(cMainmenuarray[iPosition]);
               lcd.locate(0,15);
             break;
             case 16:
             key_pressed=OK;
             break;
         }
     
        //led3= !led3;
        //printf("Task3\n");
        //vTaskDelay(1500);
    
    }*/
}
void Task4 (void* pvParameters)
{
    (void) pvParameters;                    // Just to stop compiler warnings.
    for (;;) {
        led4= !led4;
        printf("Task4\n");
        vTaskDelay(2000);
    }
}

typedef enum Color_t {
RED,
GREEN,
BLUE
} Color_t;

void initialize_led()
{
    lcd.cls();
    //iMMenu_position=0;
    lcd.locate(0,15);
    lcd.printf("Main Screen");
    lcd.locate(0,15);
         
}

int main (void)
{
    xQueue = xQueueCreate(50,sizeof(int));
    initialize_led();
    xTaskCreate( Task1, ( const char * ) "Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    // xTaskCreate( Task2, ( const char * ) "Task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    //xTaskCreate( Task3, ( const char * ) "Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    //xTaskCreate( Task4, ( const char * ) "Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    vTaskStartScheduler();
   //should never get here
   
   printf("ERORR: vTaskStartScheduler returned!");
   for (;;);
}
