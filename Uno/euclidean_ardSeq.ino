/*  
    Euclidean Arduino Sequencer Demo 
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

    This is a sketch designed for practicing how to implement Euclidean 
    algorithms musically for a step sequencer. It is in no way a finished product
    or anything unique, but I hope it can help people learn new concepts
    needed in designing sequencers with microcontrollers.

    The general idea is that we take in and format the input voltages from
    potentiometers, and use them to control a 1-9 step sequence. The potentiometers
    determine the amount of notes to distribute, the available steps to distribute to,
    a global pattern offset, and the sequence tempo. One switch transposes the sequence
    up by octaves, and the other reverses the sequence's direction.

    For better explanations of the Euclidean rhythm algorithm, visit these links:

	A definitive article on the topic
	http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf

	A practical application and code-focused explanation
	https://www.computermusicdesign.com/simplest-euclidean-rhythm-algorithm-explained/ 

	An implementation of the code to intuitively understand the algorithm
	https://dbkaplun.github.io/euclidean-rhythm/

    Controls:
    ---------
    POT1: Pattern offset
    POT2: Tempo
    POT3: Note Amount
    POT4: Step Amount
    SW1: Transpose by octaves
    SW2: Reverse direction

*/

//CODE STARTS HERE:

//Define pins

	//Control Pins - Analog Inputs
	const int tempoPin = A2;
	const int stepsPin = A5;
	const int notePin = A4;
	const int rotatePin = A3;
	const int directionPin = A1;
	const int transposePin = A0;

	//LED Pins - Digital Outputs
	const int noteOffLed = 2;
	const int noteOnLed = 3;
	int ledArray[9] = {13, 10, 7, 12, 9, 6, 11, 8, 5}; //Steps 1-9 in order

//Define variables

	int notes = 0;
	int steps = 0;
	int buffer = 0;
	int cursor = 0;
	int tempo = 0;
	int rotation = 0;
	int direction = 0;
	int transpose = 0;

	/*Currently using fixed pitches, making them variable opens up a more
	musical application, but requires more I/O and/or hardware
	Tuned roughly to A = 440Hz natural minor scale */

	int pitches[9]={440, 493, 523, 587, 659, 698, 784, 880, 987};


void setup(){

	//Set all LED pins to outputs
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

//-----Read & Format Input-----//
// In: 0-5V values
// Out: Formatted values for parameters 

	//Read step pin and divide 0-1024 by 113 to get 1-9 steps
	steps = analogRead(stepsPin)/113;

	//Set up min-max steps (0-9)
	if(steps == 0){
		steps = 1;
		}
	if(steps > 9){
		steps = 9;
		}

	//Read note pin, divide by 113 to get -9
	notes = analogRead(notePin)/113;

	//Ensure we don't have more notes than available steps
	if(notes >= steps){
		notes = steps;
		}

	//Read tempo
	tempo = analogRead(tempoPin);

	//Read the rotation pin, divide to 0-9
	rotation = analogRead(rotatePin)/113;


//-----Map Euclidean pattern-----//
//In: Notes and Steps
//Out: Step-sized array with notes evenly distributed

	//initialize an empty array of steps
	int stepState[steps];

	//Fill stepState array with Euclidean algorithm

	//Loop through all steps
	for(int i = 0; i<steps; i++){  

		//Add the amount of notes to be
		//distributed into a buffer
		buffer += notes;        

		/*If the buffer is now larger than our available steps
		we mark this as a step which contains  note and
		we subtract the the max. amount of steps from buffer
		Rinse and repeat until we find a step in which our
		buffer is equal to our max. amount of steps*/

		if(buffer >= steps){
			buffer -= steps;
			if(i+1 >= steps){
				stepState[0] = 1;	
				}
			else{
				stepState[i+1] = 1;
				}
			}

		else if(buffer < steps){
			if(i+1 >= steps){
				stepState[0] = 0;	
				}
			else{
				stepState[i+1] = 0;
				}
			}	
		}

//-----Rotate Euclidean pattern-----//
//In: Array of notes
//Out: Array of rotated notes

	//Initialize an array to store rotated steps
	int rotatedSteps[steps];

	//Loop through and apply rotation to each step and
	//move our starting array to a rotated one	
	for(int i = 0; i < steps; i++){
		rotatedSteps[i] = stepState[rotation + i];
		if((rotation + i)>=steps){
			rotatedSteps[i] = stepState[(rotation + i) - steps];
			}
	}

//-----Cycle cursor through steps-----//
//In: Array of rotated notes
//Out: LED status, note when it is set 

	//Ensure cursor doesn't go out of bounds
	if(cursor >= steps){
		cursor = 0;
		}
	if(cursor < 0){
		cursor = steps;
		if(steps == 9){
			cursor = 8;
			}
		if(steps == 1){
			cursor = 0;
			}
		}

	//Illuminate LEDs to correspond with current array
	for(int i = 0; i<steps; i++){
		if(rotatedSteps[i] != 0){
			digitalWrite(ledArray[i], HIGH);
			}
		else if(rotatedSteps[i] == 0){
			digitalWrite(ledArray[i], LOW);
			}

	//Just cosmetic bug fix for ocassionally erroneous LED[1]
	if(steps <= 1){
		digitalWrite(ledArray[1], LOW);
		}
	}

	//Clear LEDs above highest step
	for(int i = steps+1; i<9; i++){
		digitalWrite(ledArray[i], LOW);
		}

	//read pitch transpose factor
	if(analogRead(transposePin)>10){
		transpose++;
			if(transpose >= 5){ //Adjust "5" for octave range to span
				transpose = 0;
				}
		}

	//If our selected step has a note, play a tone and blink LED
	if(rotatedSteps[cursor] != 0){
		tone(4, (pitches[cursor]*pow(2,transpose)/2), 50); //Apologies for the pitch monster formula	
		digitalWrite(ledArray[cursor], LOW);
		digitalWrite(noteOnLed, HIGH);
		digitalWrite(noteOffLed, LOW);
		} 

	//If our selected step is empty, only blink LED
	if(rotatedSteps[cursor] == 0){
		digitalWrite(ledArray[cursor], HIGH);
		digitalWrite(noteOnLed, LOW);
		digitalWrite(noteOffLed, HIGH);
		} 

	//Read direction pin and set direction
	if(analogRead(directionPin)>10){
		if(direction == 0){
			direction = 1;
			}
		else if(direction == 1){
			direction = 0;
			}
		}
				
	//Step forwards or backwards
	if(direction == 0){
		cursor++;
		}
	if(direction == 1){
		cursor--;
		}

	//Determine cycle length and wait
	delay(tempo);
}
