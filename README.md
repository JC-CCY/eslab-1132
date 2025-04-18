hw-lab1:  
  
====basic problem====  

Design a program with two tasks: Task_1 and Task_2. There are two different types of blinking for the LED2, while being controlled by two tasks, respectively. When one of blinking procedure is ongoing, the other should wait for blinking to avoid interference. Namely, you need to use a mutex to protect blinking procedures.  
  
Hint: We need a semaphore and an interrupt service routine for Task_1. Task_2 need using another semphore and a timer for 10s periodically.   
  
Task_1: Press the button to trigger LED2 blinking (keep blinking in 5s with 1Hz frequency).  
  
Hint: The EXTI interrupt handler detects the button press and then inform Task_1 via a binary semaphore to blink LED.  
  
Task_2: LED2 blinks (keep blinking in 2s with 10Hz frequency) automatically every 10s.  
  
Hint: The timer interrupt handler detects the timeup event and then inform Task_2 via another binary semaphore to blink LED.  
  
=================  
  
====option problem_A====  
  
The difference from the basic problem is, you need to identify the long button press event (don't release the button over 1s). When detecting such event, Task_1 executes another blinking procedure (keep blinking in 5s with 10Hz frequency). There are two types of blinking chosen by Task_1 according to long press or normal press, so Task_1 should be informed by distinguishable messages from a queue rather than a binary semaphore.  
  
===================  

hw-lab2:    
  
====basic problem====  
  
Start/Modify a project in STM32 IoT node to read the sensor value, such as 3D Accelerator or 3D gyro, and send the data (using wifi with http or a tcp protocol) to a Linux(RPi)/Windows/Mac host and Visualize with some kind of GUI tools (such as using Python, https://mode.com/blog/python-data-visualization-libraries/Links to an external site.Links to an external site.)  
http sender example: https://github.com/iot2tangle/STM32_B-L475E-IOT01A/tree/main/http-senderLinks to an external site.   
  
=================  
  
hw-lab3:  
  
====basic problem====    
  
Write a Python BLE program (BLE Central) in a Linux host (Raspberry Pi 3) to communicate with a test app, say BLE Tool,  in an Android phone.  
  
Your BLE program is required to demo some CCCD setting, such as setting the CCCD value of your App at the Android phone to 0x0002.   
(Due to the limitations of  the "BLE Tool" app, the CCCD value might not be changed and viewed in the UI. It's OK to show the server log of the BLE Tool as a proof of your program.)  
  
The test program in your Android phone can be others, for example, BLE scanner App (by Bluepixel). iPhone probably has a similar app(By me: Lightblue can work).    
  
=================  

hw_lab4:  
  
====basic problem====  
  
In this BLE project, treat your STM32 as a server and RPi as a client. You need to build a GATT accelerator service in your STM32 with two main characteristics: (a) the 3-axis acceleration values and (b) the accelerator sampling frequency. Your RPi can assign a sampling frequency for the STM32 accelerator via writing to GATT characteristic_b. Each time when your STM32 samples acceleration data, the GATT characteristic_a is updated by new values and the STM32 notifies the new values to RPi.  
  
In STM32, please meet the requirements with RTOS by creating TASK_BLE and TASK_ACC. TASK_BLE is in charge of the basic BLE event handling for GAP connection, GAP disconnection, GATT write requests, etc. TASK_ACC is responsible for reading the accelerator values with the assigned frequency and notifying RPi.  
  
p.s. the 3-axis acceleration values should be real data sampled from LSM6DSM instead of the simulated values.  
  
=================  
  
hw-lab5:    
  
====basic problem====  
  
Please read some information about PWM techniques and applications in Internet resources (such as https://en.wikipedia.org/wiki/Pulse-width_modulation, https://electricalfundablog.com/pulse-width-modulation/, ...) and find an application that you are interested in. And generate the PWM waveforms for the applications, which can be observed using a logic analyzer. Show some waveform observations in your Demo to TA. (I choose digital communication simulation)   
  
=================    
