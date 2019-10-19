#include "C12832.h"
#include "mbed.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "DebouncedIn.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DebouncedIn joy_UP(p15); //switch
DebouncedIn joy_DOWN(p12);
DebouncedIn joy_LEFT(p13);
DebouncedIn joy_RIGHT(p16);
DebouncedIn joy_OK(p14);
//BusIn joy (p12,p13,p16,p14);

C12832 lcd(p5,p7,p6,p8,p11);
QueueHandle_t xQueue;
QueueHandle_t xQueue_LCD;
void initialize_led();
int iPosition=0;

typedef enum key_pressed_t {
NONE=0,
LEFT,
RIGHT,
UP,
DOWN,
OK
};

typedef enum fsm_state_t {
TEMPERATURE=1,
QUADRATIC,
RGB
};

void Task1 (void* pvParameters) // sender (toma el teclado como input y lo coloca en la cola. en base a eso se seleccionara la tarea siguiente)
{

    key_pressed_t key_pressed;
    BaseType_t xStatus;
    int value;
    (void) pvParameters;
    for (;;) {

        if (joy_UP.rising())
        {
            key_pressed = key_pressed_t::UP;
            xStatus=xQueueSendToBack(xQueue,(void *)&key_pressed,0);
            printf("UP\r\n");
            if (xStatus!=pdPASS) printf("Could not send to queue. \r\n\r\n");
        }
            
        if (joy_DOWN.rising())
        {
            key_pressed = key_pressed_t::DOWN;
            xStatus=xQueueSendToBack(xQueue,(void *)&key_pressed,0);
            printf("DOWN\r\n");
            if (xStatus!=pdPASS) printf("Could not send to queue. \r\n");
        } 
        if (joy_LEFT.rising())
        {
            key_pressed = key_pressed_t::LEFT;
            xStatus=xQueueSendToBack(xQueue,(void *)&key_pressed,0);
            printf("LEFT\r\n");
            if (xStatus!=pdPASS) printf("Could not send to queue. \r\n\r\n");
        } 
        if (joy_RIGHT.rising())
        {
            key_pressed = key_pressed_t::RIGHT;
            xStatus=xQueueSendToBack(xQueue,(void *)&key_pressed,0);
            printf("RIGHT\r\n");
            if (xStatus!=pdPASS) printf("Could not send to queue. \r\n");
        }
        if (joy_OK.rising())
        {
            key_pressed = key_pressed_t::OK;
            xStatus=xQueueSendToBack(xQueue,(void *)&key_pressed,0);
            printf("OK\r\n");
            if (xStatus!=pdPASS) printf("Could not send to queue. \r\n");
        }        
    vTaskDelay(100);
    } 
}
void Task2 (void* pvParameters) //receiver - state machine
{
    key_pressed_t key_pressed;
    fsm_state_t state,prev_state;
    const TickType_t xCycleFrequency = pdMS_TO_TICKS( 100UL );
    BaseType_t xStatus;

  int len;
   
    state = TEMPERATURE;
    xStatus=xQueueSendToBack(xQueue_LCD,(void *)&state,0);
    if (xStatus!=pdPASS) printf("Could not send to queue. \r\n");
    prev_state = TEMPERATURE;
    
    (void) pvParameters;
    for (;;) {
     
    // if (uxQueueMessagesWaiting (xQueue)!=0)
    // {
        // lcd.printf("Queue should have been empty!\r\n"); 
    // }
    // char strLCD[100];
        len  = (int)uxQueueSpacesAvailable(xQueue);

        // printf("task2 len=%d\r\n",len);
   
   xStatus = xQueueGenericReceive(xQueue, (void *)&key_pressed, xCycleFrequency,pdFALSE);
   if (xStatus==pdPASS)
   {
     printf("TASK 2 - entro dato al queue\r\n");  
      switch (state)  // TEMP<->QUAD<->RGB
        {
             case fsm_state_t::TEMPERATURE:
             {
                 if (key_pressed == key_pressed_t::LEFT )
                 {  
                     state = fsm_state_t::RGB;
                 }
                 else if ( key_pressed == key_pressed_t::RIGHT)
                 {
                     state = fsm_state_t::QUADRATIC;
                 }
                 else
                 {
                    state = fsm_state_t::TEMPERATURE;
                 }
            }
            break;
            case fsm_state_t::QUADRATIC:
            {
                 if (key_pressed == key_pressed_t::LEFT )
                 {  
                     state = fsm_state_t::TEMPERATURE;
                 }
                 else if ( key_pressed == key_pressed_t::RIGHT)
                 {
                     state = fsm_state_t::RGB;
                 }
                 else
                 {
                    state = fsm_state_t::QUADRATIC;
                    /* code */
                 }           
             }
             break;
             case fsm_state_t::RGB:
             {
                if (key_pressed == key_pressed_t::LEFT )
                 {  
                     state = fsm_state_t::QUADRATIC;
                 }
                 else if ( key_pressed == key_pressed_t::RIGHT)
                 {
                     state = fsm_state_t::TEMPERATURE;
                 }
                 else
                 {
                    state = fsm_state_t::RGB;
                    /* code */
                 }
             }
             break;  

             default:
            {
                state = TEMPERATURE;
            }
             break;           
        

        }
        if(prev_state=state){
         xStatus=xQueueSendToBack(xQueue_LCD,(void *)&state,0);
            if (xStatus!=pdPASS) printf("Could not send to queue.\r\n");
            printf("envio al LCDDDDDDDDDDD\r\n");

        }
        
        prev_state=state;

      }
    //   printf("Estado %d\r\n",(char)state);
     //   led2= !led2;
     //   printf("Task2\n");
       vTaskDelay(100);
    }
}
void Task3 (void* pvParameters) //Menu printer
{
    fsm_state_t state;
    const TickType_t xCycleFrequency = pdMS_TO_TICKS( 100UL );
    BaseType_t xStatus;
    int len;
    // char strLCD[100];
    for (;;) {
        len  = (int)uxQueueSpacesAvailable(xQueue_LCD);

        printf("task3\r\n");
        
        xStatus = xQueueGenericReceive(xQueue_LCD, (void *)&state, xCycleFrequency,pdFALSE);
        if (xStatus==pdPASS)
        {
            
            switch (state)
            {
            case fsm_state_t::TEMPERATURE: 
            {
            //    lcd.cls();
            //    //iMMenu_position=0;
            //    lcd.locate(0,3);
            //    lcd.printf("Temperature Screen");
            //    wait(0.1);
            //    lcd.locate(0,15);
            printf("task3 - temp\r\n");
            }
            break;

            case fsm_state_t::QUADRATIC: 
            {
            //     lcd.cls();
            //     //iMMenu_position=0;
            //     lcd.locate(0,15);
            //     lcd.printf("QUADRATIC Screen");
            //     lcd.locate(0,15);
            printf("task3 - quad\r\n");
            }
            break;

            case fsm_state_t::RGB: 
            {
            //     lcd.cls();
            //     //iMMenu_position=0;
            //     lcd.locate(0,15);
            //     lcd.printf("RGB Screen");
            //     lcd.locate(0,15);
            printf("task3 - rgb\r\n");
            }
            break;

            default:
                break;
            }

        }

        vTaskDelay(250);
    }
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
    xQueue = xQueueCreate(10,sizeof(key_pressed_t));
    xQueue_LCD = xQueueCreate(10,sizeof(fsm_state_t));
    // initialize_led();
    xTaskCreate( Task1, ( const char * ) "Task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    xTaskCreate( Task2, ( const char * ) "Task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, ( xTaskHandle * ) NULL );
    xTaskCreate( Task3, ( const char * ) "Task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    //xTaskCreate( Task4, ( const char * ) "Task4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
    vTaskStartScheduler();
   //should never get here
   
   printf("ERORR: vTaskStartScheduler returned!\r\n");
   for (;;);
}
