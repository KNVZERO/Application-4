/* --------------------------------------------------------------
   Application: 04 - Rev1
   Name: Kelvin Vu
   Release Type: Use of Memory Based Task Communication
   Class: Real Time Systems - Fall 2025
   AI Use: Please commented inline where you use(d) AI
---------------------------------------------------------------*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"

//TODO 8 - Update the code variables and comments to match your selected thematic area!

//TODO 0a connect the components in the diagram to the GPIO pins listed below.
//Note even if Wokwi won't blow your LEDs, we'll assume them to be off if they're not connected to resistors!
//Next, goto TODO 0b
#define LED_GREEN GPIO_NUM_5
#define LED_RED   GPIO_NUM_4
#define BUTTON_PIN GPIO_NUM_18
#define POT_ADC_CHANNEL ADC1_CHANNEL_6 // GPIO34

#define MAX_COUNT_SEM 10 
// TODO 7: Based ont the speed of events; 
//can you adjust this MAX Semaphore Counter to not miss a high frequency threshold events
//over a 30 second time period, e.g., assuming that the sensor exceeds the threshold of 30 seconds, 
//can you capture every event in your counting semaphore? what size do you need?


// Threshold for analog sensor
//TODO 1: Adjust threshold based on your scenario or input testing
// You should modify SENSOR_THRESHOLD to better match your Wokwi input behavior;
// note the min/max of the adc raw reading

// 3276 is about 80% of the max value
// It will be the critical threshold for "radiation levels"
#define SENSOR_THRESHOLD 3276

// Handles for semaphores and mutex - you'll initialize these in the main program
SemaphoreHandle_t sem_button;
SemaphoreHandle_t sem_sensor; // Recheck this for counting
SemaphoreHandle_t print_mutex;

volatile int SEMCNT = 0; //You may not use this value in your logic -- but you can print it if you wish

//TODO 0b: Set heartbeat to cycle once per second (on for one second, off for one second)
//Find TODO 0c
void system_task(void *pvParameters) {
    bool led_on = false;
    TickType_t currentTime = pdTICKS_TO_MS( xTaskGetTickCount() );

    while (1) {
      //currentTime = pdTICKS_TO_MS( xTaskGetTickCount() );
      gpio_set_level(LED_GREEN, led_on); // set LED state to bool led_on
      led_on = !led_on;  // toggle state for next time
      //printf("Status LED %s @ %lu \n", led_on ? "ON" : "OFF", currentTime);
      vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 ms using MS to Ticks Function vs alternative which is MS / ticks per ms

    }
}


void sensor_task(void *pvParameters) {

    const TickType_t periodTicks = pdMS_TO_TICKS(100); // e.g. 100 ms period
    TickType_t lastWakeTime = xTaskGetTickCount(); // initialize last wake time

    while (1) {
        int val = adc1_get_raw(POT_ADC_CHANNEL);

        //TODO 2: Add serial print to log the raw sensor value (mutex protected)
        //Hint: use xSemaphoreTake( ... which semaphore ...) and printf
        if (xSemaphoreTake(print_mutex, portMAX_DELAY) == pdTRUE) { // Takes Mutex
          
          //May comment this printf out because it filled the console
          printf("Radiation Level is %d [%.2f%%]!\n", val, ((float)val / 4095) * 100);

          xSemaphoreGive(print_mutex); // Returns Mutex
        }

        //printf("Value: %d\n", val);        

        //TODO 3: prevent spamming by only signaling on rising edge; See prior application #3 for help!
        static bool prev_above = false;  // remembers if the last reading was above threshold
        bool curr_above = (val > SENSOR_THRESHOLD);

        if (curr_above && !prev_above) {
            // Rising edge detected: value just crossed the threshold upward{
            if(SEMCNT < MAX_COUNT_SEM+1) SEMCNT++; // DO NOT REMOVE THIS LINE
            
            //printf("Radiation Level is %d [%.2f%%]!\n", val, ((float)val / 4095) * 100);
            
            xSemaphoreGive(sem_sensor);  // Signal sensor event

        }
        // Update previous state for next iteration
        prev_above = curr_above;



        vTaskDelayUntil(&lastWakeTime, periodTicks);
        //vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void button_task(void *pvParameters) {

   static bool previous_state = false;

    while (1) {
        int state = gpio_get_level(BUTTON_PIN);
        bool current_state = (state == 0); // active-low button
        // TODO 4a: Add addtional logic to prevent bounce effect (ignore multiple events for 'single press')
        // You must do it in code - not by modifying the wokwi simulator button

        if (current_state && !previous_state) {
            xSemaphoreGive(sem_button);

          if (xSemaphoreTake(print_mutex, portMAX_DELAY) == pdTRUE) {
              printf("Button Press Detected! \n");
              xSemaphoreGive(print_mutex);
          }

            // optional debounce delay of arbitrary time
            vTaskDelay(pdMS_TO_TICKS(200)); //Comment this to break the system
        }

        //Comment this to break the system
        previous_state = current_state; // update previous state
            
        //Comment this to break the system
        vTaskDelay(pdMS_TO_TICKS(10)); // Do Not Modify This Delay!
    }
}

void event_handler_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(sem_sensor, 0)) {
            SEMCNT--;  // DO NOT MODIFY THIS LINE

            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("Threshold Exceeds Safe Levels!\n");
            xSemaphoreGive(print_mutex);

            gpio_set_level(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_RED, 0);
        }

        if (xSemaphoreTake(sem_button, 0)) {
            xSemaphoreTake(print_mutex, portMAX_DELAY);
            printf("System Alert Triggered!\n");
            xSemaphoreGive(print_mutex);

            gpio_set_level(LED_RED, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
            gpio_set_level(LED_RED, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Idle delay to yield CPU
    }
}

void app_main(void) {
    // Configure output LEDs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GREEN) | (1ULL << LED_RED),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // Configure input button
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&btn_conf);

    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POT_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Create sync primitives
    // TODO 0c: Attach the three SemaphoreHandle_t defined earlier 

    // Asked ChatGPT for help regarding which attachment to which semaphore and verified it through the RTOS textbook

    sem_button = xSemaphoreCreateBinary();
    sem_sensor = xSemaphoreCreateCounting(MAX_COUNT_SEM,0);
    print_mutex = xSemaphoreCreateMutex();

    // (sem_button, sem_sensor, print_mutex) to appropriate Semaphores.
    // binary, counting, mutex by using the appropriate xSemaphoreCreate APIs.
    // the counting semaphore should be set to (MAX_COUNT_SEM,0);
    // Move on to TODO 1; remaining TODOs are numbered 1,2,3, 4a 4b, 5, 6 ,7
 


    //TODO 5: Test removing the print_mutex around console output (expect interleaving)
    //Observe console when two events are triggered close together
    //Removing the mutex breaks the system


    // Create tasks
    xTaskCreate(system_task, "System Status", 2048, NULL, 1, NULL);
    xTaskCreate(sensor_task, "System Sensor", 2048, NULL, 2, NULL);
    xTaskCreate(button_task, "System Button", 2048, NULL, 3, NULL);
    xTaskCreate(event_handler_task, "Event_Handler", 2048, NULL, 2, NULL);

    //TODO 6: Experiment with changing task priorities to induce or fix starvation
    //E.G> Try: xTaskCreate(sensor_task, ..., 4, ...) and observe heartbeat blinking
    //You should do more than just this example ...
}
