/*  
Euclidean Arduino Clock
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

*/

/*define I/O pins*/
const int led[8] = {2,3,4,5,6,7,8,9};
const int pot1 = A7;
const int pot2 = A6;
const int sw1 = A5;
const int sw2 = A4;
const int trigOn = 12;
const int trigOff = 11;

/*define variables*/
const int maxSteps = 8;
int steps = 8;
int notes = 2;
int rotation = 0;
int buffer = 0;
int cursor = 0;
int tempo = 500;
int trigOnLen = 0;
int trigOffLen = 0;
bool cursorDir = true;
bool changeTempo = true;
bool changeSteps = false;
bool changeNotes = false; 
bool changeDir = true;
bool pattern[8] = {0};

void setup(){
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(12, OUTPUT);
}

/*Functions for reading and scaling potentiometer input*/
int readTempo(int pot){
	int ret = (analogRead(pot) - 1024)*(-1);
	return ret;
	}

int readSteps(int pot){
	int ret = analogRead(pot) / 120;
	return ret;
	}

int readRotation(int pot){
	int ret = ((analogRead(pot) - 1024)*(-1))/120;
	return ret;
	}

bool swOn(int sw){
/*A function for reading switch state*/
	if(analogRead(sw) > 10){
		return true;
	} else {
		return false;
	}
}	

void readControls(){
/*A functionf or reading user input*/

	/*Physical controls must revisit their set values 
	 *to adjust the value, as to avoid sudden changes 
	 *when toggling modes of operation*/
	
	if (abs(readTempo(pot1) - tempo) < 10){
		changeTempo = true;
	}
	if (abs(readSteps(pot1) - steps) < 1){
		changeSteps = true;
	}	
	if (abs(readSteps(pot1) - notes) < 1){
		changeNotes = true;
	}	

	/*Read which button combination is pressed, and toggle
	 *mode accordingly, then read value*/
	
	if (swOn(sw1) == false && swOn(sw2) == false){

		changeSteps = false;
		changeNotes = false;
		changeDir = true;

		if (changeTempo == true){
			tempo = readTempo(pot1); 
		}
	}

	if (swOn(sw1) == true && swOn(sw2) == false){

		changeTempo = false;
		changeNotes = false;		

		if (changeSteps == true){
			steps = readSteps(pot1); 
			if (steps <= 0){
				steps = 1;
			}	
		}
	}

	if (swOn(sw2) == true && swOn(sw1) == false){

		changeTempo = false;
		changeSteps = false;

		if (changeNotes == true){
			notes = readSteps(pot1); 
			if (notes > steps){
				notes = steps;
			}
		}
	}

	if (swOn(sw1) == true && swOn(sw2) == true){

		if(changeDir == true){
			if (cursorDir == true){
				cursorDir = false;
			} else {
				cursorDir = true;
			}
			changeDir = false;
		}
	}

	rotation = readRotation(pot2); 
}

void writePattern(bool* pattern, int steps, int notes){
/*A function for calculating Euclidean distribution*/

	/*Loop through each step, and add our "notes" amount
	 *into a buffer with each step. Every time our buffer
	 *buffer "overflows", we mark it as a step which contains 
	 *a note, subtract our total steps and repeat until the 
	 *buffer is filled without overflow*/

	for (int i = 0; i<steps; i++){

		buffer += notes;
		if (buffer >= steps){
			buffer -= steps; 

			/*Our pattern is naturally offset by 1 so
			 *so that we always have a note on the 1st
			 *beat. Here we check if we are to set the
			 *first or i+1:th step as a note-step*/

			if(i+1 >= steps){ 
				pattern[0] = true;
			} else {
				pattern[i+1] = true;
			}
		} else {
			if (i+1 >= steps){
				pattern[0] = false;
			} else {
				pattern[i+1] = false;
			}
		}
	}
}

void rotatePattern(bool* pattern, int steps, int rotation){
/*A function for applying rotation to Euclidean distribution*/

	/*We loop through our pattern and write, apply
	 *rotation, and write it to a buffer. Once rotated
	 *we overwrite the pattern from the buffer*/

	bool buffer[steps];
        for (int i = 0; i < steps; i++){
                buffer[i] = pattern[rotation + i];
                if ((rotation + i)>=steps){
                        buffer[i] = pattern[(rotation + i) - steps];
		}
       	}
	for (int i = 0; i < steps; i++){
		pattern[i] = buffer[i];
	}
}

void writeLed(bool* pattern, int steps){
/*A function for displaying pattern on LEDs*/
	for (int i = 0; i<steps; i++){
		if (pattern[i] == false){
			digitalWrite(led[i], LOW);
		}
		if (pattern[i] == true){
			digitalWrite(led[i], HIGH);
		}
	}
}

void writeCursor(bool* pattern, int cursor){
/*A function for writing the cursor state onto the
 *the Euclidean pattern*/
	if (pattern[cursor] == true){
		digitalWrite(led[cursor], LOW);
		if (steps <= 1){
			digitalWrite(led[cursor], HIGH);
		}
	} else if (pattern[cursor] == false){
		digitalWrite(led[cursor], HIGH);
	} 
}

void clearLed(){
/*A function for clearing stray LEDs*/

	for (int i = 0; i < maxSteps; i++){
		digitalWrite(led[i], LOW);
	}
}

int moveCursor(bool* pattern, int cursor, int steps, bool cursorDir){
/*A function for moving the cursor forwards or backwards
 *within a certain set min-max range*/

	if (cursorDir == true){
		cursor++;
	} else if (cursorDir == false){
		cursor--;
	}

	if (cursor >= steps){
		cursor = 0;
	} else if (cursor < 0){
		cursor = steps-1;
	}
	return cursor;
}
	
void writeTrig(bool* pattern, int cursor){
/*A function for sending a trigger out on note steps*/
	if (pattern[cursor] == true){
		digitalWrite(trigOn, HIGH);
	}
	if (pattern[cursor] == false){
		digitalWrite(trigOff, HIGH);
	}
}

void clearTrig(bool* pattern, int cursor){
/*A function for clearing the trigger output after
 *certain trigger length*/
	if (trigOnLen == 10){
		digitalWrite(trigOn, LOW);
		trigOnLen = 0;
	}
	if (trigOffLen == 10){
		digitalWrite(trigOff, LOW);
		trigOffLen = 0;
	}
	trigOnLen++;
	trigOffLen++;
}
void loop(){

	for (int i = 0; i < tempo; i++){
		readControls();
		writePattern(pattern, steps, notes);
		rotatePattern(pattern, steps, rotation);
		writeLed(pattern, steps);
		writeCursor(pattern, cursor);
		clearTrig(pattern, cursor);
	}
	
	writeTrig(pattern, cursor);
	cursor = moveCursor(pattern, cursor, steps, cursorDir);
	clearLed();
}
