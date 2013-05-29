/*
 Copyright (C) 2013  Peter Lotts

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <Arduino.h>
#include <stl_config.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "digi_write.h"
#include "notedefs.h"

Shifter shifts2(SHIFT_SERIAL, SHIFT_LATCH, SHIFT_CLOCK, 1 /*Number of shift registers in chain*/);
digi_pins pins(&shifts2,"0,1");
digi_rf rf(&pins, RF_IN_BIT_1, RF_IN_BIT_2, RF_IN_BIT_3, RF_IN_BIT_4, RF_IN_INTER, RF_OUT_BIT_1, RF_OUT_BIT_2, RF_OUT_BIT_3, RF_OUT_BIT_4);
digi_serial com(&pins, RF_OUT_BIT_1, RF_IN_BIT_1, RF_IN_INTER);

#if (DRONE == 1)
	digi_batt batt(&pins,BATT_ALERT);
	digi_lcd lcd(&pins,&batt,LCD_REG_SELECT,LCD_ENABLE,LCD_D4,LCD_D5,LCD_D6,LCD_D7);
	void rf_is_up();
	void launch();
	// Default is left (false)
	bool joystick_t_f(digi_lcd *screen, digi_pins *pins, String top_message, String options);
	void newdata();
	String tmp="";
	void newdata()
	{
		/*if ((pins.read(6) == pins.read(11)) && (pins.read(5) == pins.read(10)) && (pins.read(4) == pins.read(9)) && (pins.read(3) == pins.read(8)))
		{
			pins.set(13,true);
		}
		else
		{
			pins.set(13,false);
		}*/
		tmp+=(char)(48+pins.read(6));
		tmp+=(char)(48+pins.read(5));
		tmp+=(char)(48+pins.read(4));
		tmp+=(char)(48+pins.read(3));
		if (tmp.length() == 8)
		{
			Serial.println(tmp + " Done receiving");
			tmp="";
		}
	}

	void setup()
	{
		//lcd.addbuzzer(BUZZER);
		//lcd.buzz(1000,0,1);
		//batt.setup(MAX17043_ALERT_PERCENT);
		//lcd.begin(16,2);
		//lcd.write_row("DS CCF Drone",false) -> write_row("Initializing...",true);
		// Perform initial setup here

		//bool gps_needed=joystick_t_f(&lcd,&pins,"Enable GPS?"," No        Yes "); // Maybe add in function processing?
		//rf.activate(rf_is_up,&lcd,gps_needed); // Get it to set up the system


		//lcd.write_row("Primed",false) -> write_row("Turn on Drone",true);
		//delay(2000);
		pins.setio(6,true)->setio(5,true)->setio(4,true)->setio(3,true)->setio(2,true)->setio(11,true)->setio(10,true)->setio(9,true)->setio(8,true)->interrupt(2,newdata,RISING);
		pins.setio(13,false);
		Serial.begin(9600);
		Serial.print((char)27);
		Serial.print("[2J");
		Serial.println("Startup");
	}
	void rf_is_up()
	{
		com.activate();
		lcd.write_row("Connected",false) -> write_row("Press to launch",true);
		pins.interrupt(BUTTON_LAUNCH,launch,RISING);
	}
	void launch()
	{
		com.write("Launch");
		lcd.write_row("Launching...",true);
		String haslaunched;
		com.read(&haslaunched,8);
		lcd.write_row("Restrictions Enabled",false) -> write_row("Manual Control",true);
	}

	void loop()
	{
		/*//batt.update(); lcd.update(); delay(1000);
		char num[10];
		dtostrf((float)analogRead(0) * 0.0049,0,2,num);
		String mil(millis(),10);
		lcd.write_row((String)num + "V " + mil + "s",false); delay(10);

		batt._timer.update();*/
	}

	bool joystick_t_f(digi_lcd *screen, digi_pins *pins, String top_message, String options)
	{
		screen->switch_data() ->write_row(top_message,false) ->write_row(options,true) ->setCursor(true,1) ->cursor() ->blink();
		pinMode(JOYSTICK_1_CLICK,INPUT);
		bool ret=false;
		int an;
		while (1)
		{
			an=analogRead(JOYSTICK_1_H);
			if (an >= 816 /* 4V / 0.0049 */)
			{
				screen->setCursor(13,true);
				ret=true;
			}
			else if (an <= 204 /* 1V / 0.0049 */)
			{
				screen->setCursor(1,true);
				ret=false;
			}

			if (digitalRead(JOYSTICK_1_CLICK) == HIGH)
			{
				break;
			}
			delay(10);
		}
		screen->switch_data() ->noCursor() ->noBlink();
		return ret;
	}
#else

/*// Minimal class to replace std::vector
template<typename Data>
class Vector {
   size_t d_size; // Stores no. of actually stored objects
   size_t d_capacity; // Stores allocated capacity
   Data *d_data; // Stores data
   public:
     Vector() : d_size(0), d_capacity(0), d_data(0) {}; // Default constructor
     Vector(Vector const &other) : d_size(other.d_size), d_capacity(other.d_capacity), d_data(0) { d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); }; // Copy constuctor
     ~Vector() { free(d_data); }; // Destructor
     Vector &operator=(Vector const &other) { free(d_data); d_size = other.d_size; d_capacity = other.d_capacity; d_data = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(d_data, other.d_data, d_size*sizeof(Data)); return *this; }; // Needed for memory management
     void push_back(Data const &x) { if (d_capacity == d_size) resize(); d_data[d_size++] = x; }; // Adds new value. If needed, allocates more space
     size_t size() const { return d_size; }; // Size getter
     Data const &operator[](size_t idx) const { return d_data[idx]; }; // Const getter
     Data &operator[](size_t idx) { return d_data[idx]; }; // Changeable getter
   private:
     void resize() { d_capacity = d_capacity ? d_capacity*2 : 1; Data *newdata = (Data *)malloc(d_capacity*sizeof(Data)); memcpy(newdata, d_data, d_size * sizeof(Data)); free(d_data); d_data = newdata; };// Allocates double the old space
};*/

	struct note
	{
		int freq;
		int time;
		note(int hz, int rel_time)
		{
			freq=hz;
			time=rel_time;
		}
	};
	struct tune
	{
		int delay_ms;
		std::vector<note> notes;
		tune(int ms_between, std::vector<note> thenotes)
		{
			delay_ms=ms_between;
			notes=thenotes;
		}
	};
	Timer _tunetimer;
	int _tunepin;
	int _tuneindex;
	String _tunename;
	std::map<String,tune> tunes;
	std::vector<note> avect;
	
	void tune_worker()
	{
		if (_tuneindex < tunes[_tunename].notes.size())
		{
			tone(_tunepin,tunes[_tunename].notes[_tuneindex].freq);
			_tunetimer.after(tunes[_tunename].notes[_tuneindex].time,tune_worker);
			_tuneindex++;
		}
		else
		{
			noTone(_tunepin);
			_tuneindex=0;
		}
	}
	void playtune(String tune_name, int pin)
	{
		avect.push_back(note(note_A4,2));
		avect.push_back(note(note_C4,1));
		avect.push_back(note(note_D4,1));
		avect.push_back(note(note_E4,2));
		avect.push_back(note(note_E4,1));
		avect.push_back(note(note_F4,1));
		avect.push_back(note(note_A4,1));
		avect.push_back(note(note_G4,1));
		avect.push_back(note(note_A4,1));
		avect.push_back(note(note_G4,1));
		avect.push_back(note(note_A4,1));
		avect.push_back(note(note_G4,3));
		tunes["Dad's Army"]=tune(100,avect);

		_tunename=tune_name;
		_tunepin=pin;
		_tuneindex=0;
		tune_worker();
	}

	void setup()
	{
		pins.setio(31,false)->setio(35,false)->setio(39,false)->setio(43,false);
		Serial.begin(9600);
		Serial.print((char)27);
		Serial.print("[2J");
		Serial.println("Startup");
		playtune("Dad's Army",4);
		for (int x=0; x < 14*100; x++)
		{
			_tunetimer.update();
		}
	}

	char nextval[10];
	int x, wait=50;
	void loop()
	{
		x=Serial.readBytesUntil('\n',nextval,10);
		if (x >= 8)
		{
			pins.set(43,(nextval[0] == 49));
			pins.set(39,(nextval[1] == 49));
			pins.set(35,(nextval[2] == 49));
			pins.set(31,(nextval[3] == 49));
			Serial.println("Written data part 1: "+(String)(nextval[0]-48)+(String)(nextval[1]-48)+(String)(nextval[2]-48)+(String)(nextval[3]-48));
			delay(wait);
			pins.set(43,false);
			pins.set(39,false);
			pins.set(35,false);
			pins.set(31,false);

			delay(wait);
			pins.set(43,(nextval[4] == 49));
			pins.set(39,(nextval[5] == 49));
			pins.set(35,(nextval[6] == 49));
			pins.set(31,(nextval[7] == 49));
			Serial.println("Written data part 2: "+(String)(nextval[4]-48)+(String)(nextval[5]-48)+(String)(nextval[6]-48)+(String)(nextval[7]-48));
			delay(wait);
			pins.set(43,false);
			pins.set(39,false);
			pins.set(35,false);
			pins.set(31,false);
		}
	}
#endif