# Synchronization Quest - Application 4 

## Real Time Systems - Summer 2025
**Instructor:** Dr. Mike Borowczak

## Author
**Name:** Kelvin Vu

**Theme:** Space Systems 

---

## Project Overview

This project explores task coordination and shared resource protection using FreeRTOS on the ESP32. The system simulates real-time sensor monitoring and alert handling using three key synchronization mechanisms:

- **Binary Semaphore** - signals from a user pushbutton
- **Counting Semaphore** - queues threshold events from an analog sensor
- **Mutex** - protects shared access to the serial console output

**Hardware Setup (Wokwi simulation):**

- 1x Potentiometer → GPIO34 (Analog Sensor)
- 1x Pushbutton → GPIO18
- 1x Red LED → GPIO4 (Alert)
- 1x Green LED → GPIO5 (Heartbeat)

---

## Engineering Analysis

Answer the following based on your project implementation and observations:

### 1. Signal Discipline (Binary Semaphore)
How does the binary semaphore synchronize the button press with the system? What did you observe when pressing the button quickly? Why is a binary semaphore appropriate here?

**Answer:**  
The binary semaphore acts as a key that the button task creates. When the button is pressed
a binary semaphore is created, which the event task takes. When pressing the button quickly,
it still acted normally, perhaps due to the debouncing section. Assuming that the button bounced,
it would create multiple semaphores and cause some mishaps in the system. A binary semaphore is 
approriate here because it is the easiest to implement for safety. It essentially boils down to
Task A gives a semaphore, and Task B receives that semaphore. 

---

### 2. Event Flood Handling (Counting Semaphore)
What happens when the potentiometer crosses the threshold multiple times rapidly? How does the counting semaphore handle that? What would break if you used a binary semaphore instead? How did you tune the max count in the semaphore to capture a 30 second event flood (TODO 7)?

**Answer:**  
When the potentiometer crosses the threshold multiples time rapidly, it gives the 
semaphore and the event task activates, triggering the red LED and print statement.
It would consume a slot everyime it crossed over the threshold.
The counting semaphore allows tasks to access the same resource, provided that
it is below the maximum count. It sort of acts like a queue for the data read.
If a binary sempahore was used instead, it would not be fast enough to process the task. 
It would only give a semaphore for the very first instance because it would need to process it,
then need another event to trigger the semaphore. The max count in the semaphore should match
the rate at which the data is read. The sensor task checks every 100 ms, for a max count
of 10. Given the same ration, for a 30 second event flood, the max count should be 
changed to 300. 

---

### 3. Protecting Shared Output (Mutex)
Which shared resource is protected by the mutex? What, if anything, happened when you removed it? What does this reveal about mutual exclusion in FreeRTOS?

**Answer:**  
The shared resource being protected by the mutex is the print for the console.
When the mutex is removed, it allows tasks to interweave their print outputs. Mutual 
exclusion in FreeRTOS is important for allowing only one task access a resource at a time.


---

### 4. Scheduling and Preemption
Describe how task priorities influenced scheduling (TODO 6). Provide an example where a high-priority task preempted a lower one. What happened to the heartbeat during busy periods?

**Answer:**  
The priorities for the tasks from highest priority to least was the button, followed by
sensor and event, then the heartbeat. When a semaphore is given to the higher priority
task, it can preempt a lower priority one. The button task has the highest priority,
so when it is pressed, it always preempts the other tasks. During busy period, the
heartbeat should slow down or stop due to the other tasks consuming the CPU 
processing. Perhaps due to how I coded the tasks, the heartbeat still displays a 
constant rhythm. 

---

### 5. Timing and Responsiveness
The code provided uses `vTaskDelay` rather than `vTaskDelayUntil`. How did delays impact system responsiveness and behavior? Does the your polling rate affect event detection? Would you consider changing any of the `vTaskDelay` rather than `vTaskDelayUntil` - why or why not? Adjust your code accordingly.

**Answer:**  
For most of the tasks, vTaskDelay is sufficient enough for non-specified timings. 
Whenever the LED states were switched, such as in the heartbeat and event tasks, the 
actual length was not that important. Delays impact the system by shifting the all
the tasks over, essentially causing drift. If a delay of 100 ms occured, the sequential
tasks would be off by that 100 ms delay. The polling rate does affect event detection,
the sensor polls every 100 ms, so any values that cross the threshold would be detected
within 100 ms. If a larger polling value was used, it could miss events if they were 
too quick. The sensor task should be changed to vTaskDelayUntil if we want specified timings
for the system, which would be exactly every 100 ms.

---

### 6. Theme Integration
Relate each component of your system to your chosen theme. For example, what does the sensor represent in a space probe? How does synchronization reflect safety requirements?

**Answer:**  
For this application, I chose the space theme. The heartbeat task represents the 
running state of the system, which is shown by a flashing LED. If any errors occur in the system,
it is shown by the flash length or lack thereof. The sensor task represents the readings
of cosmic radiation. The actual threshold value chosen a bit arbarbitrary due to how the
potentiometer goes from 0-4095. A percentage value was used.
It reads the value and logs it. The button task represents an 
external event that is activated by the user. It has the highest priority so it always
runs, hence it can act as an alert button. The event task represents reading the 
sensor readings and button presses. It returns a message in response to these events. 
Synchronization shows that all the tasks can run together and properly. 

---

### 7. [Bonus] Induced Failure - Starvation or Loss of Responsiveness
Did you design an experiment to break the system (e.g., starving the heartbeat task or missing button presses)? What did you observe? Include the modified (commented-out) code in your Wokwi project.

**Answer:**  
In the button task, editing the debouncing system and a few delays starves the 
sensor task and event task out due to priorities. This only happens if the button
is held down. The only output in the console is from the button task and none from the 
sensor or event task. 

---

## Presentation Slides

Link to your 4-slide summary here (google slides, onedrive powerpoint):
https://ucf-my.sharepoint.com/:p:/g/personal/ke088292_ucf_edu/EalKBRwcau9Bg0F1v_EhoawBsfb_E3u12Ugdw-JMretyWQ?e=UzGYwT

1. Introduction and Theme
2. Most Important Technical Lesson
3. Favorite Part of the Project
4. Something That Challenged You or You'd Explore More

(Bonus, Optional) If you included a voiceover, describe how to access it or link to a video. 

Voiceover link included in the Powerpoint.

---

## Summary
You’re not just coding — you’re building a real-time system. Each semaphore is a signal. Each mutex is a lock guarding safety. Each LED pulse is a message from your system’s heartbeat.
Can you keep your events ordered, your resources safe, and your system timely? This is your synchronization quest.
Good luck.

**Final Wokwi Project Link:** https://wokwi.com/projects/445802050930148353

Download the project Zip.
Head over to webcourses
