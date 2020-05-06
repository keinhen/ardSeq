/*  
    Programmable Linear Arduino Sequencer Demo 
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

    This is a sketch designed for practicing how to make a programming
    interface for a linear step sequencer. It is in no way a finished product
    or anything unique, but I hope it can help people learn new concepts
    needed in designing sequencers with microcontrollers.

    The general idea is that we take in and format the input voltages from
    potentiometers, and use them to control a 1-9 step sequence. By pressing
    both switches you toggle between the two available
    modes, SEQUENCER MODE (freerunning playback of the programmed sequence) and
    PROGRAMMER MODE (selecting a step and programming values to said step).

    Two knobs allow you to set global sequence length (1-9 steps) and vary the
    playback tempo. Two other knobs affect Pitch and Note Length. In PROGRAMMER MODE
    you use these to program their respective values, and in SEQUENCER MODE you
    use these to apply a global shift to their programmed values, Pitch in octaves
    up, and Length shifting to much shorter or much longer.

    Controls	Programmer mode		Sequencer mode
    --------	---------------		---------------
    POT1: 	Set pitch		Transpose pitch
    POT2: 	Set note length		Scale global note length
    POT3: 	Sequence tength		Sequence length
    POT4: 	Sequence tempo		Sequence tempo
    SW1+SW2:	Go to Sequencer Mode 	Go to Programmer Mode
    SW1: 	Step backwards		n/a
    SW2: 	Step forwards		n/a
*/

//CODE STARTS HERE:

//Define pins

	//Control pins - Analog Inputs 
	const int tempoPin = A5;
	const int stepsPin = A4;
	const int pitchPin = A3;
	const int lengthPin = A2;
	const int fwPin = A1;
	const int bwPin = A0;
	
	//Led pins - Digital Outputs
	const int progModeLed= 2;
	const int seqModeLed= 3;
	int ledArray[9] = {13, 10, 7, 12, 9, 6, 11, 8, 5}; //Steps 1-9 in order

//Define variables

	//Sequence variables	
	int tempo = 0;
	int steps = 0;
	int cursor = 0;
	
	//Note pitch variables
	float pitchIn = 0;
	int pitchRef = 0;
	int semitone = 0;
	float factor = 0;
	float pitchOut = 0; 
	int transpose = 0;

	//Initially tuned to A=440Hz natural minor scale, 9 steps
	int notes[9]={440, 493, 523, 587, 659, 698, 784, 880, 987};

	//Note length variables
	int readLength = 0;
	int newLength = 0;
	float scale = 1;
	int length[9]={100, 100, 100, 100, 100, 100, 100, 100, 100};

	//Navigation variables
	int fwState = 0;
	int bwState = 0;
	int toggleState = 0;
	int progMode= 0;
	int seqMode = 1;


void setup(){
	
	//Set all LED pins to outputs
	pinMode(13, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(7, OUTPUT);
	pinMode(12, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(6, OUTPUT);
	pinMode(11, OUTPUT);
	pinMode(8, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(2, OUTPUT);
	}

void loop(){

//-----Read and format inputs-----//
//In: 0-5V values
//Out: Formatted values for parameters

	//Read tempo and steps
	tempo = analogRead(tempoPin);
	steps = analogRead(stepsPin)/110;

	//Read and store the navigation pin states
	if(analogRead(fwPin) > 10){
		fwState = 1;
		}
	else// if(analogRead(fwPin <= 10)){
	{	fwState = 0;
		}
	if(analogRead(bwPin) > 10){
		bwState = 1;
		}
	else// if(analogRead(bwPin <= 10)){
	{	bwState = 0;
		}

	//If both buttons are pressed toggle between modes 
	if(fwState == 1 && bwState == 1){
		toggleState = 1;
		}
	else{
		toggleState = 0;
		}

	//When changing states from Sequencer mode
	if(toggleState == 1 && seqMode == 1){
		seqMode = 0;
		progMode = 1;
		delay(500); //Adjust for mode change "sensitivity"
		}

	//When changing states from Programmer mode
	else if(toggleState == 1 && progMode == 1){
		progMode = 0;
		seqMode = 1;
		delay(500); //Adjust for mode change "sensitivity"
		}

//-----Programming mode-----//
//Pauses playback, allows step-programming
//Exit by pressing both buttons, navigate by single buttons
//When at a step, turn pitch or length knobs to set value

	if(progMode == 1){

		//Switch mode, indicate with LED
		seqMode = 0;
		digitalWrite(progModeLed, HIGH);
		digitalWrite(seqModeLed, LOW);	
	
		//-----Program Pitch-----//
		//Check if pitch is being changed
		//If it is, quantize input voltage to
		//chromatic scale, then store new pitch	
	
			//Read current pitch
			//Delay by a desired sample size
			//Read new pitch for a sample of difference
			pitchIn = analogRead(pitchPin);
			if(pitchIn<= 0){
				pitchIn= 1;
				}

			delay(20);     //Adjust this for time-based "sensitivity" of pitch knob
			pitchRef = analogRead(pitchPin);	

			//Quantize input frequency to 2 octaves starting from 220Hz
		
			/*How to tune:
				- Adjust "42" to desired divider for pitch range
				e.g. Here 1024/42 = ~24.380, which spans 2 octaves when rounded down.  
				- Adjust "220" to desired root frequency in Hz*/
					
			semitone = pitchIn/42;
			factor = pow(2, (float)semitone/12);
			pitchOut = 220*factor;  
		
			//Measure if there was enough difference to indicate a control change
			//If threshold is crossed, store new quantized pitch and play it
			//Adjust "3" for voltage-based "sensitivity of pitch knob
			if(abs(pitchIn - pitchRef) > 3){
				notes[cursor] = pitchOut;
				tone(4, notes[cursor], length[cursor]);
				}
		
		//-----Program Length-----//
		//Check if length is being changed
		//If it is, set the note length to new value

			//Read current length
			//Delay by a desired sample size
			//Read new length for a sample of difference 
			readLength = analogRead(lengthPin);
			if(readLength <= 0){
				readLength = 1;
				}
			delay(20);  //Adjust this for time-based "sensitivity" of length knob		
			newLength = analogRead(lengthPin);

			//Mesure if there was enough difference to indicate a control change
			//If threshold is crossed, store new length and play it
			//Adjust "3" for voltage-based "sensitivity" of length knob 
			if(abs(newLength - readLength) > 3){

				//Store a new scaled length into step
				//Not super happy with the scaling formula, let me know
				//if you come up with anything more responsive!
				length[cursor] = newLength/3;
				if(length[cursor] == 0){
					length[cursor] = 1;
					}	

				//Play new note 
				tone(4, notes[cursor], length[cursor]);
				}
	
		//If forwards button is pressed, move to the next step
		if(fwState == 1 && bwState == 0){
			cursor++;
			if(cursor>= steps){
				cursor = 0;
				}
			//Clear the led array	
			for(int i = 0; i<9; i++){
				digitalWrite(ledArray[i], LOW);
					}

			//Write the new step to led array and play it
			digitalWrite(ledArray[cursor], HIGH);
			tone(4, notes[cursor], length[cursor]);
			delay(100);
			}

		//If backwards button is pressed, move to the previous step 
		if(bwState == 1 && fwState == 0){
			cursor--;
			if(cursor< 0){
				cursor= steps-1;
				}
			//Clear the led array	
			for(int i = 0; i<9; i++){
				digitalWrite(ledArray[i], LOW);
					}
			//Write the new step to led array and play it
			digitalWrite(ledArray[cursor], HIGH);
			tone(4, notes[cursor], length[cursor]);
			delay(100);		
			}
	}
		
//-----Sequencer mode-----//
//Cycles through steps stored in array
//Plays back pitch, length and LED position stored

	if(seqMode == 1){ 

		//Switch mode, indicate  with led	
		progMode == 0;	
		digitalWrite(seqModeLed, HIGH);
		digitalWrite(progModeLed, LOW);

		//Clear the led array	
		for(int i = 0; i<9; i++){
			digitalWrite(ledArray[i], LOW);
				}

		//Read a transposition factor for live octave transpose 
		transpose = pow(2, (analogRead(pitchPin)/204));

		//Read a scale factor for live not length scaling
		scale = (analogRead(lengthPin)/100);
		if(scale <= 0){
			scale = 1;
			}

		//Write the current step to led array and play tone
		digitalWrite(ledArray[cursor], HIGH);
		tone(4, ((notes[cursor]*transpose)), length[cursor]*(1/scale));

		//Count upwards and wait according to tempo
		cursor++;
		if(cursor>= steps){
			cursor = 0;
			}
		delay(tempo);

	}

//Reset control state
fwState = 0;
bwState = 0;
}
