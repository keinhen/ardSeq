/*  
    Cartesian Arduino Sequencer Demo 
    Copyright (C) 2020 Henry Keinälä
    henry.keinala@tuta.io

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    <https://www.gnu.org/licenses/>.

    ------

    This is a sketch designed for practicing how to implement Cartesian movement 
    musically for a step sequencer using two clocks. It is in no way a finished product 
    or anything unique, but I hope it can help people learn new concepts 
    needed in designing sequencers with microcontrollers. 
 
    The general idea is that we take in and format the input voltages from 
    potentiometers, and use them to control a 1-9 step sequence, clocked by two
    3 step sequences. Each time the X-sequence steps forward, the cursor moves either
    left or right, and conversely each tim the Y-sequence steps forward, the cursor
    moves either up or down.

    The potentiometers determine the rate of each individual sequence, and the length
    of the sequence in total. The buttons toggle the direction of each sequence. 

    The two clocks are built in a bit of a strange way, and I believe this sketch would work
    much better with analog clocks like 555/556 that can be truly independent. But with this
    workaround we can get this working with minimal external circuitry, and still achieve a fun
    chaotic sequencer.

    Controls:
    ---------
    POT1: X-sequence length 
    POT2: Y-sequence length
    POT3: X-sequence tempo
    POT4: Y-sequence tempo
    SW1: X-sequence direction
    SW2: Y-sequence direction

*/

//CODE STARTS HERE:

//Define pins

	//Control pins for X sequence
	const int xOutPin = 2;
	const int xInPin = A5;
	const int xDirPin = A1;
	const int xLenPin = A2;

	//Control pins for Y sequence
	const int yOutPin = 3;
	const int yInPin = A4;
	const int yDirPin = A0;
	const int yLenPin= A3;

	//LED Array set up as a 2-dimensional array	
	const int ledArray[3][3]={{13, 10, 7},{12, 9, 6},{11, 8, 5}};

//Define variables

	//Variables for X-sequence	
	int xRate = 0;
	int xLen = 0;
	int xCount = 0;
	int xState = 0;
	int xPos = 0;
	int xDir = 0;

	//Variables for Y-sequence
	int yRate = 0;
	int yLen = 0;
	int yCount = 0;
	int yState = 0;
	int yPos = 0;
	int yDir = 0;

        /*Currently using fixed pitches, making them variable opens up a more
        musical application, but requires more I/O and/or hardware
        Tuned roughly to A=440Hz natural minor scale */

	int pitches[3][3]={{440, 493, 523},{587, 659, 698},{784, 880, 987}};

void setup(){

	//Set all LED pins to outputs
	pinMode(xOutPin, OUTPUT);
	pinMode(yOutPin, OUTPUT);
	pinMode(13, OUTPUT);
	pinMode(12, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(2, OUTPUT);
	}

void loop(){

//-----Read and format inputs-----//
//In: 0-5v values
//Out: Formatted values for parameters

	//Read and scale clock rates
	xRate = analogRead(xInPin)/2;
	yRate = analogRead(yInPin)/2;

	//Read and scale sequence lengths
	xLen = analogRead(xLenPin)/341;
	yLen = analogRead(yLenPin)/341;	

	//Read direction pins, toggle if HIGH
	//For X-sequence
	if(analogRead(xDirPin) > 10){
		if(xDir == 0){
			xDir = 1;
			}
		else if(xDir == 1){
			xDir = 0;
			}
		}

	//For Y-sequence
	if(analogRead(yDirPin) > 10){
		if(yDir == 0){
			yDir = 1;
			}
		else if(yDir == 1){
			yDir = 0;
			}
		}

//-----Clocks-----//
//In: Clock rates
//Out: 50% duty cycle, xRate*2 frequency square wave

   //Note: here instead of dealing with delay() or reading ext clocks we count 
   //to xRate to determine where we are within the squarewave's progression
   //Once xRate == xCount we are at the edge of the wave, and we need to change state
   //from HIGH to LOW or vice versa at the next step, and begin counting up again 
   //When going from HIGH to LOW we are starting a new cycle, so we move the cursor
   //By adjusting values here you could achieve PWM or other waveform effects

		//If the pulse is HIGH, turn on LED
		//Once cycle reaches edge of square wave, toggle state
		//and reset counter
		if(xState == 1){
			digitalWrite(xOutPin, HIGH);
			if(xCount >= xRate){
				digitalWrite(xOutPin, LOW);
				xState = 0;
				xCount = 0;
				}
			}

		//If the pulse is LOW, turn off LED
		//Once cycle reaches of edge of square wave, toggle state,
		//reset counter and move the cursor in x direction
		else if(xState == 0){
			digitalWrite(xOutPin, LOW);
			if(xCount >= xRate){
				digitalWrite(xOutPin, HIGH);
				xState = 1;
				xCount = 0;
				if(xDir == 0){	
					xPos++;
					}
				if(xDir == 1){
					xPos--;
					}
				}
			} 

		//Same as above, but for the Y-clock
		if(yState == 1){
			digitalWrite(yOutPin, HIGH);
			if(yCount >= yRate){
				digitalWrite(yOutPin, LOW);
				yState = 0;
				yCount = 0;
				}
			}

		else if(yState == 0){
			digitalWrite(yOutPin, LOW);
			if(yCount >= yRate){
				digitalWrite(yOutPin, HIGH);
				yState = 1;
				yCount = 0;
				if(yDir == 0){
					yPos++;
					}
				if(yDir == 1){
					yPos--;
					}
				}
			} 

//-----Step movement-----//
//In: Current (x,y) position 
//Out: LED on and tone at correct coordinates

	//Clear array of steps 
	for(int k = 0; k<3; k++){
		for(int l = 0; l<3; l++){
			digitalWrite(ledArray[l][k], LOW);
			}
		}

	//Determine max boundaries for array 
	if(xPos > xLen){
		xPos = 0;
		}
	if(xPos < 0){
		xPos = 2;
		}
	if(yPos > yLen){
		yPos = 0;
		}	
	if(yPos < 0){
		yPos = 2;
		}

	//Toggle current X,Y step, light LED and play tone
	digitalWrite(ledArray[xPos][yPos], HIGH);
	tone(4,pitches[xPos][yPos], 10);

	//Count forwards 
	xCount++;
	yCount++;

}
