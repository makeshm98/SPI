# SPI
SPI funcitonalities

### 1. Page Byte and Register Address:
Think of a page in a book. Each page has many lines, right? In computers, memory is divided into pages, and each page is like a page in a book. Now, within each page, there are small sections called registers, just like paragraphs in a book. Each register has its own address, just like each paragraph in a book has a number. So, if you want to find something in a book, you go to the right page and then find the paragraph you need using its number. Similarly, in computers, if you want to find or change something in memory, you need to go to the right page and then find the register by its address.

### 2. SPI (Serial Peripheral Interface):
SPI is like a way for different parts of a computer to talk to each other, like friends talking on the phone. Imagine you have a toy robot that needs to talk to a controller to know what to do next. The SPI is like the phone line connecting the robot and the controller. They use this line to send messages back and forth quickly.

### 3. SPI Run, Read, and Write:
- **SPI Run:** When you start the SPI, it's like turning on the phone line. Now the robot and the controller can talk.
  
- **SPI Read:** This is like listening to what the other person is saying on the phone. The robot might ask the controller, "What should I do next?" and then listen for the answer.
  
- **SPI Write:** This is like talking on the phone. The controller might say to the robot, "Move forward," and the robot will do what it's told.

### Real World Example:
Imagine you have a robot car controlled by a remote. The remote (controller) and the car (robot) need to communicate to make the car move. Each command the remote sends (like 'Go Forward' or 'Turn Left') is like a message sent over SPI. The car's computer (microcontroller) reads these messages and tells the motors what to do.

So, when you press a button on the remote (SPI Write), it sends a message to the car (SPI Read). The car then understands the message and moves accordingly (SPI Run).

I hope this helps you understand! Let me know if you have more questions!

## Pin Connection 
ESPDevKitV1   BME688 
- D5          -   CS
- D18         -   SCK
- D19         -   SD0
- D23         -   SDI
