//Drink Mixer Code
//Jeff Lai
typedef struct{
		float adjust;
		int flavourDirection;
		int flavourTimer;
		int numShakes;
		int drumNum;
} flavour;
//checks if the touch sensor is pressed
bool isCupThere(int touchPort)
{
	if (SensorValue[touchPort] == 1)
	{
		eraseDisplay();
		return true;
	}
	else
	{
		displayString(1, "Please put a cup.");
		return false;
	}
}

//checks if touch sensor is pressed
//changes variable to true if it is pressed
void eStop(bool& isPressed, int touchPort)
{
	if(SensorValue[touchPort] == 1 || isCupThere(S2) == false)
		isPressed = true;
	else
		isPressed = false;
}

//the tank is empty if the ultrasonics sensor
//reads a distance greater than 10
//10 was decided through experimentation
bool isTankEmpty(int ultrasonicPort)
{
	if ((SensorValue[ultrasonicPort]) > 10)
		return true;
	return false;
}

void turnOnPump(bool isOn)
{
	if(isOn)
		motor[motorA] = 100;
	else
		motor[motorA] = 0;
}

void turnOnDispenser(bool isOn, int percent, flavour f, bool& emergencyStop, int speed)
{
	//White drum half closer to motor: Cherry
	//White drum half further from motor: Orange
	//White drum controlled by motor B
	//Orange drum half closer to motor: Grape
	//Orange drum half further from motor: Peach
	//Orange drum controlled by motor C
  //cherry 0, grape 1, orange 2, peach 3
	const float ENC_LIMIT = 3*360/(2*PI*3);
	// these two flavours turn the same direction on their
	//respective drums, and so if either flavour is chosen
	//the direction is reversed
	// checks that the percent is not 0 and the motor is turned on
	if (isOn && percent != 0)
	{
			clearTimer(T1);
			nMotorEncoder[motorB] = nMotorEncoder[motorC] = 0;

			//Rotote drum to the dispensing hole
			// motor keeps moving in a direction until it reaches the encoder limit
			// or if emergency stop is pressed
			while((abs(nMotorEncoder[motorB]) < (ENC_LIMIT) && !emergencyStop)
			 && (abs(nMotorEncoder[motorC]) < (ENC_LIMIT) && !emergencyStop))
			{
				if(f.drumNum == 1)
					motor[motorB] = speed*f.flavourDirection;
				else
					motor[motorC] = speed*f.flavourDirection;
				eStop(emergencyStop, S3);
			}

			//Wait set amount of time to dispense powder

			//Shake the drums back and forth a certain number of times
			for(int counter = 0; counter < f.numShakes; counter++)
			{
				while(time1(T1)<(percent/100*(f.flavourTimer/f.numShakes))
				 && !emergencyStop)
				{
					motor[motorB] = motor[motorC] = 0;
					eStop(emergencyStop, S3);
				}
				time1(T1) = 0;
				nMotorEncoder[motorB] = nMotorEncoder[motorC] = 0;
				while((abs(nMotorEncoder[motorB]) < (1.5*(ENC_LIMIT)) 
				&& !emergencyStop) && (abs(nMotorEncoder[motorC]) < 
				(1.5*(ENC_LIMIT)) && !emergencyStop))
				{
					if(f.drumNum == 1)
						motor[motorB] = -speed*f.flavourDirection;
					else
						motor[motorC] = -speed*f.flavourDirection;
					eStop(emergencyStop, S3);
				}
				nMotorEncoder[motorB] = nMotorEncoder[motorC] = 0;
				while((abs(nMotorEncoder[motorB]) < (1.5*(ENC_LIMIT)) 
				&& !emergencyStop) && (abs(nMotorEncoder[motorC]) < 
				(1.5*(ENC_LIMIT)) && !emergencyStop))
				{
					if(f.drumNum == 1)
						motor[motorB] = speed*f.flavourDirection;
					else
						motor[motorC] = speed*f.flavourDirection;
					eStop(emergencyStop, S3);
				}
			}
			//motors stay still for a time depending on the percent given
			//2500 was used as the number after experimental testing
			while(time1(T1)<(percent/100*(f.flavourTimer/f.numShakes)) 
			&& !emergencyStop)
			{
				motor[motorB] = motor[motorC] = 0;
				eStop(emergencyStop, S3);
			}

			//Return drum back to original position
			nMotorEncoder[motorB] = nMotorEncoder[motorC] = 0;
			while((abs(nMotorEncoder[motorB]) < (f.adjust*ENC_LIMIT) 
			&& !emergencyStop) && (abs(nMotorEncoder[motorC]) < 
			(f.adjust*ENC_LIMIT) && !emergencyStop))
			{
				if(f.drumNum == 1)
					motor[motorB] = -speed*f.flavourDirection;
				else
					motor[motorC] = -speed*f.flavourDirection;
				eStop(emergencyStop, S3);
			}
			motor[motorB] = motor[motorC] = 0;
		}
	else
	 	motor[motorB] = motor[motorC] = 0;
}

int selectPercentage()
{
	int selectionNum = 2, percentage = 0;
	bool doneSelecting = false;
	do {
		displayString(0, " Press the up and down buttons");
		displayString(1, " to select a percentage");
		displayString(2, " 0");
		displayString(3, " 25");
		displayString(4, " 50");
		displayString(5, " 75");
		displayString(6, " 100");
		//displays a "-" on the percentage being selected
		displayString(selectionNum, "-");
		while(!getButtonPress(buttonAny))
		{}
		if (getButtonPress(buttonDown))
		{
			while(getButtonPress(buttonAny))
			{}
			if(selectionNum < 6)
				selectionNum++;
		}
		else if(getButtonPress(buttonUp))
		{
			while(getButtonPress(buttonAny))
			{}
			if(selectionNum > 2)
				selectionNum--;
		}
		else if(getButtonPress(buttonEnter))
		{
			while(getButtonPress(buttonAny))
			{}
			if (selectionNum <= 6 && selectionNum >= 2)
				percentage = (selectionNum-2) * 25;
			doneSelecting = true;
		}
	} while(!doneSelecting);
	eraseDisplay();
	//returns the appropriate percentage based on what the user selects
	return percentage;
}

void selectFlavour(int* percentValue)
{
	int selectionNum = 2, totalPercentage = 0;
	bool doneSelecting = false, not100Percent = false, waterLevelOK = false, 
	notFullTank = false;

	do {
		displayString(0, " Press the up and down buttons");
		displayString(1, " to select a flavour");
		displayString(2, " Cherry");
		displayString(3, " Grape");
		displayString(4, " Orange");
		displayString(5, " Peach");
		displayString(6, " Done Selecting");
		displayString(selectionNum, "-");
		displayString(7, "Total: %d percent", totalPercentage);
		displayString(8, "Cherry: %d percent", percentValue[0]);
		displayString(9, "Grape: %d percent", percentValue[1]);
		displayString(10, "Orange: %d percent", percentValue[2]);
		displayString(11, "Peach: %d percent", percentValue[3]);
		if(not100Percent)
		{
			displayString(12, "Please select percentages that");
			displayString(13, "add to 100 percent");
		}
		not100Percent = false;
		if(notFullTank)
			displayString(14, "Tank is not full, please refill.");
		notFullTank = false;
		while(!getButtonPress(buttonAny))
		{}
		if (getButtonPress(buttonDown))
		{
			while(getButtonPress(buttonAny))
			{}
			if(selectionNum < 6)
				selectionNum++;
		}
		else if(getButtonPress(buttonUp))
		{
			while(getButtonPress(buttonAny))
			{}
			if(selectionNum > 2)
				selectionNum--;
		}
		else if(getButtonPress(buttonEnter))
		{
			while(getButtonPress(buttonAny))
			{}
			if (selectionNum <= 5 && selectionNum >= 2)
			{
				eraseDisplay();
				percentValue[selectionNum - 2] = selectPercentage();
				totalPercentage = 0;
				for(int counter = 0; counter < 4; counter++)
					totalPercentage += percentValue[counter];
			}
			//checks if the "done selecting" option is pressed
			else if (selectionNum == 6)
			{
				//checks if total percentage is 100
				if (totalPercentage == 100)
					doneSelecting = true;
				else
					not100Percent = true;
				if (!isTankEmpty(S1))
					waterLevelOK = true;
				else
					notFullTank = true;
			}
		}
	} while(!doneSelecting || !waterLevelOK);
	eraseDisplay();
}

const int FILL_TIME = 45000;

task main()
{
	const int FLAVOUR_NUM = 4;
	int percentValue[FLAVOUR_NUM] = {0,0,0,0};
	bool emergencyStop = false;
	flavour cherry;
	cherry.adjust = 1.5;
	cherry.flavourDirection = -1;
	cherry.flavourTimer = 8000;
	cherry.numShakes = 1;
	cherry.drumNum = 1;
	flavour grape;
	grape.adjust = 1.5;
	grape.flavourDirection = 1;
	grape.flavourTimer = 8000;
	grape.numShakes = 1;
	grape.drumNum = 1;
	flavour orange;
	orange.adjust = 1.45;
	orange.flavourDirection = -1;
	orange.flavourTimer = 60000;
	orange.numShakes = 50;
	orange.drumNum = 2;
	flavour peach;
	peach.adjust = 1.4;
	peach.flavourDirection = 1;
	peach.flavourTimer = 26500;
	peach.numShakes = 25;
	peach.drumNum = 2;
	flavour allFlavours[FLAVOUR_NUM] = {cherry, grape, orange, peach};
	SensorType[S1] = sensorEV3_Ultrasonic;
	SensorType[S2] = sensorEV3_Touch;
	SensorType[S3] = sensorEV3_Touch;
	selectFlavour(percentValue);
	while(!isCupThere(S2)){}
	turnOnPump(true);
	clearTimer(T2);
	for (int counter = 0; counter < FLAVOUR_NUM; counter++)
		turnOnDispenser(true, percentValue[counter], allFlavours[counter],
		 emergencyStop, 100);
	while(time1(T2) < FILL_TIME && (!emergencyStop))
	{
		eStop(emergencyStop, S3);
	}
	turnOnPump(false);
	turnOnDispenser(false, 0, cherry, emergencyStop, 100);
	if (time1(T1) > FILL_TIME)
		playSound(soundBeepBeep);
	else
		playSound(soundLowBuzz);
	wait1Msec(5000);
}
