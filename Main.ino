
#define set_pin A0        // RF pin
#define scoop_pin A1
#define rotate_pin A3

#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial wifi(8,7);
Servo arm_servo, spoon_servo;


const int buzzer_ = 4;          // buzzer pin
const int data_pin = 8;         // RF pin
const int arm_pin = 5;          // Servo <arm> pin
const int spoon_pin = 6;        // Servo <spoon> pin
const int motor_pin = 9;        // Motor pin
const int magnet_sensor = 12;     // Optical sensor
const int optical_sensor = 11;      // Magnet sensor
const int transfer_speed = 1024;    // RF speed of transmission
const int lib_smallest_pulse = 510;     // time of pulse for 0 degree
const int lib_biggest_pulse = 2570;   // time of puls for 180 degree
const int upper_arm_limit_angle = 145;    // max position for setting << arm >>
const int lower_arm_limit_angle = 100;    // min ....
bool first_run = true;
bool first_scoop_auto = true;
const int init_ = 0;          // init value for all
							  // converting pulse to angle for visualizing
int angle(int angle)
{
	return map(angle, 0, 180, lib_smallest_pulse, lib_biggest_pulse);
}
const long arm_home_pos = angle(20);  // Home position of servo 1
const long spoon_home_pos = angle(45);   // this value is unknown, need to test // Home position of servo 2
										 // interval of increasement or decreasement of pusle 


// interval
int interval_arm_pos = 25;
int interval_spoon_pos = 25;


long current_arm_pos = arm_home_pos; // ?  current position of servo2
long current_spoon_pos = angle(0);
int upper_arm_limit_pos = angle(upper_arm_limit_angle);
int lower_arm_limit_pos = angle(lower_arm_limit_angle);
int velocity_arm = 0;
int velocity_spoon = 0;


const long arm_pos_for_sco = angle(30);
const long spoon_scoop	=  angle(30);
//const long spoon_pos_for_sco = angle(110);  // postiion of ending spinning the spoon for sco
volatile bool get_mess = false;
bool inv_ = false;              // Inverse for adjusting in setting arm position mode
char food[3] = { '#A', '#B', '#C' };
struct Bowl_id { int Pa, Pb, Pc; };
unsigned int pos_of_bowl = 0;
struct Bowl_id bowl_id = { 0,1,2 };


void setup()
{
	pinMode(buzzer_, OUTPUT);
	pinMode(set_pin, INPUT_PULLUP);
	pinMode(scoop_pin, INPUT_PULLUP);
	pinMode(rotate_pin, INPUT_PULLUP);
	pinMode(arm_pin, INPUT);
	spoon_servo.attach(spoon_pin);
	spoon_servo.writeMicroseconds(spoon_home_pos);
	pinMode(spoon_pin, INPUT);
	arm_servo.attach(arm_pin);
	arm_servo.writeMicroseconds(arm_home_pos);
	//pinMode(spoon_pin, OUTPUT);
	pinMode(motor_pin, OUTPUT);
	pinMode(optical_sensor, INPUT);
	pinMode(magnet_sensor, INPUT);
	Serial.begin(9600);
	Serial.println("READY");
	
	
	Home(); // initial position
	
	//BTSerial.begin(9600);
	wifi.begin(9600);
}
//void blue_tooth(char a)
//{
//	if (BTSerial.available())    // read from HC-05 and send to Arduino Serial Monitor
//		Serial.write(a);
//}

void buzzer(int times, int frequency)
{
	if (frequency == 0)
		frequency = 1200;
	for (int i = 0; i < times; i++)
	{
		tone(buzzer_, frequency);
		delay(500);
		noTone(buzzer_);
	}
}
void spinning_tray()
{
	digitalWrite(motor_pin, HIGH);
	Serial.println("tray");
	delay(300);
	while (digitalRead(magnet_sensor) == 1);
	delay(100);
	digitalWrite(motor_pin, LOW);
	count_bowl();
}
void count_bowl()
{
	pos_of_bowl++;
	if (pos_of_bowl > 2)
	{
		pos_of_bowl = 0;
	}
}
void Home_motor()
{
	if (digitalRead(optical_sensor) != 1)
	{
		digitalWrite(motor_pin, HIGH);
		while (digitalRead(optical_sensor) == 0);
		digitalWrite(motor_pin, LOW);
	}
}
void Home()
{
	buzzer(1, 1500);
	current_arm_pos = arm_home_pos;
	current_spoon_pos = spoon_home_pos;
	arm_servo.writeMicroseconds(arm_home_pos);
	spoon_servo.writeMicroseconds(spoon_home_pos);
	Home_motor();
}
volatile int set()
{
	if (digitalRead(set_pin) == 1)
		return 1;
	else
		return 0;
}
volatile int scoop()
{
	if (digitalRead(scoop_pin) == 1)
		return 1;
	else
		return 0;
}
volatile int rotate()
{
	if (digitalRead(rotate_pin) == 1)
		return 1;
	else
		return 0;
}
void set_for_auto()
{

	while (current_arm_pos < upper_arm_limit_pos)
	{
		current_arm_pos += interval_arm_pos;
		current_spoon_pos += interval_spoon_pos;
		arm_servo.writeMicroseconds(current_arm_pos);
		spoon_servo.writeMicroseconds(current_spoon_pos);
		delay(55);
	}

}
void set_pos()
{
	// over upper-> inverse

	velocity_arm = 50;
	//Serial.println("start ");
	first_run = false;
	while (set() == 0)
	{
		if (inv_ == false)
		{
			if (current_arm_pos < upper_arm_limit_pos)
			{
				current_arm_pos += interval_arm_pos;
				current_spoon_pos += interval_spoon_pos;
				arm_servo.writeMicroseconds(current_arm_pos);
				spoon_servo.writeMicroseconds(current_spoon_pos);
				delay(velocity_arm);
				inv_ = false;
			}

			else
			{
				inv_ = true;
				delay(1500);
			}
		}
		if (inv_ == true)
		{
			if (current_arm_pos > lower_arm_limit_pos)
			{
				current_arm_pos -= interval_arm_pos;
				current_spoon_pos -= interval_spoon_pos;
				arm_servo.writeMicroseconds(current_arm_pos);
				spoon_servo.writeMicroseconds(current_spoon_pos);
				delay(velocity_arm);
				inv_ = true;
				//Serial.println(current_arm_pos);  //
				//Serial.println(data());       // 
			}
			else
			{
				inv_ = false;
				delay(1500);
			}
		}
		Serial.println("set");

	}

}


void scooping()
{
	velocity_arm = 50;
	long var_spoon_current = current_spoon_pos;
	long temp_arm_pos = 0;
	// 
	//Serial.println(pos_of_bowl);
	// bringing food to user


	// arm vs spoon
	for (int i = current_arm_pos; i >arm_pos_for_sco; i -= 25)
	{
		temp_arm_pos = i;
		arm_servo.writeMicroseconds(temp_arm_pos);
		delay(50);
		var_spoon_current -= 25;
		spoon_servo.writeMicroseconds(var_spoon_current);
	}
	delay(200);
	//  down the spoon
	for (int i = var_spoon_current; i >= spoon_scoop; i -= 20)
	{
		spoon_servo.writeMicroseconds(i);
		delay(velocity_arm);
	}
	// arm goes lowest pos for scooping
	for (int i = temp_arm_pos; i > angle(20); i -= 20)
	{
		arm_servo.writeMicroseconds(i);
		delay(velocity_arm);
	}
	// scooped food
	for (int i = spoon_scoop; i < spoon_home_pos; i += 20)
	{
		spoon_servo.writeMicroseconds(i);
		delay(velocity_arm);
	}
	for (int i = temp_arm_pos; i < arm_home_pos; i += 20)
	{
		arm_servo.writeMicroseconds(i);
		delay(velocity_arm);
	}
	/*for (int i = var_spoon_current; i >= angle(40); i -= 50)
	{
	spoon_servo.writeMicroseconds(i);
	delay(20);
	}*/
	for (int i = 0; i < 2; i++)
	{
		spoon_servo.writeMicroseconds(angle(35));
		delay(800);
		spoon_servo.writeMicroseconds(spoon_home_pos);
		delay(800);
	}
	int var_arm_current = arm_home_pos;
	//  if (current_arm_pos < lower_arm_limit_pos)
	//  {
	//    current_arm_pos = upper_arm_limit_pos;
	//    current_spoon_pos = angle(170);
	//  }
	var_spoon_current = spoon_home_pos;
	for (int i = arm_home_pos; i < current_arm_pos*0.8; i += 10)
	{
		var_spoon_current += 10;
		if (var_spoon_current < current_spoon_pos)
		{
			spoon_servo.writeMicroseconds(var_spoon_current);
		}
		delay(30);
		arm_servo.writeMicroseconds(i);
	}
	for (int i = current_arm_pos*0.8; i < current_arm_pos; i += 11)
	{
		var_spoon_current += 12;
		if (var_spoon_current < current_spoon_pos)
		{
			spoon_servo.writeMicroseconds(var_spoon_current);
		}
		delay(40);
		arm_servo.writeMicroseconds(i);
	}
	delay(40);
	arm_servo.writeMicroseconds(current_arm_pos);
	spoon_servo.writeMicroseconds(current_spoon_pos);
	wifi.println(food[pos_of_bowl]);
}

String auto_mode2()
{
	String text = "";
	if (Serial.available() > 0) {
		if (Serial.read() == '%')
		{
			text = Serial.readString();
			text.trim();
			get_mess = true;
		}
		Serial.println(text);
		Serial.println(text.length());

		return text;
	}
}


/*String auto_mode()
{
	if (BTSerial.available()>0) {

		String text = "";
		if (BTSerial.read() == '%')
		{
			text = BTSerial.readString();
			text.trim();
			get_new = true;
		}
		Serial.println(text);
		return text;
	}

}
*/
String auto_mode_wifi()
{
	String text = "";
	text = wifi.readString();
	if (wifi.read() == '%')
	{
		text = wifi.readString();
		get_mess = true;
	}
	Serial.println(text);
	return text;
}


int Max(int a, int b, int c)
{
	int max_ = a;
	if (b > max_)
	{
		max_ = b;

	}
	if (c > max_)
	{
		max_ = c;
	}
	return max_;

}
struct  auto_data { int fa, fb, fc, max_; };
struct auto_data setting_auto(String st)
{
	String s1, s2, s3;
	int len = st.length(), a, b, c;
	//st.toCharArray(s,st.length());
	if (st != 0)
	{
		Serial.println(st.charAt(0));
		for (int i = 0; i < len; i++)
		{
			Serial.println(st.charAt(i));
			delay(20);
			if (st.charAt(i) == 'A')
			{
				a = i + 1;
			}
			else if (st.charAt(i) == 'B')
			{
				b = i + 1;
			}
			else if (st.charAt(i) == 'C')
			{
				c = i + 1;
			}

		}
		Serial.println(a);
		Serial.println(b);
		Serial.println(c);
		int j = a;
		while (st.charAt(j) != 'B')
		{
			s1 += st.charAt(j); j++;
		}
		int val1 = s1.toInt();
		Serial.println(val1);
		j = b;
		while (st.charAt(j) != 'C')
		{
			s2 += st.charAt(j); j++;
		}
		int val2 = s2.toInt();
		Serial.println(val2);
		j = c;
		while (st.charAt(j) != '\0')
		{
			s3 += st.charAt(j); j++;
		}
		int val3 = s3.toInt();
		Serial.println(val3);
		int max_ = Max(val1, val2, val3);
		Serial.println(Max(val1, val2, val3));
		return{ val1,val2,val3,max_ };
	}
}

void serveA()
{
	Serial.println("Food A");

	while (pos_of_bowl != bowl_id.Pa)
	{
		spinning_tray();
		delay(500);
	}
	Serial.println(pos_of_bowl);
	delay(1000);
	scooping();


}
void serveB()
{
	Serial.println("Food B");

	while (pos_of_bowl != bowl_id.Pb)
	{
		spinning_tray();
		delay(200);
	}
	Serial.println(pos_of_bowl);
	delay(1000);
	scooping();

}
void serveC()
{
	Serial.println("Food C");

	while (pos_of_bowl != bowl_id.Pc)
	{
		spinning_tray();
		delay(200);
	}
	Serial.println(pos_of_bowl);
	delay(1000);
	scooping();

}
void notify()
{
	Serial.println("wait_");

}
void auto_()
{
	String s;
	int time_to_eat = 4000;
	s = auto_mode_wifi();
	delay(2);
	if (s != "")
	{
		struct auto_data auto_ = setting_auto(s);
		if (first_run == true)
		{
			Serial.println(first_run);
			set_for_auto();
			first_run = false;
			delay(1000);
		}
		Serial.print("max= ");
		Serial.println(auto_.max_);
		if (auto_.max_ > 0)
		{
			delay(100);
			while (auto_.max_ > 0)
			{
				if (auto_.fa > 0)
				{
					serveA();
					Serial.println("a");
					auto_.fa -= 1;
				}
				delay(time_to_eat);
				if (auto_.fb > 0)
				{
					serveB();
					Serial.println("b");
					auto_.fb -= 1;

				}
				delay(time_to_eat);
				if (auto_.fc > 0)
				{
					serveC();
					Serial.println("c");
					auto_.fc -= 1;
				}
				delay(time_to_eat);
				auto_.max_ -= 1;
			}
			get_mess = false;
			Serial.println("finish auto");
		}

	}
}
	

void run_auto()
{
	String s;
	if (get_mess == false)
	{
		s = auto_mode_wifi();
		Serial.println(s);
	}
	int time_to_eat = 2000;
	//char s1[] ="",s2[] = "",s3[] = ""  ;
	if (get_mess == true)
	{
		if (first_run == true)
		{
			Serial.println(s);

			set_for_auto();

			first_run = false;
			delay(1000);
		}
		Serial.println(s);
		setting_auto(s);
		delay(2);
		struct auto_data auto_ = setting_auto(s);
		Serial.print("max= ");
		Serial.println(auto_.max_);
		if (auto_.max_ > 0)
		{
			delay(100);
			while (auto_.max_ > 0)
			{

				if (auto_.fa > 0)
				{
					serveA();
					//spinning_tray();
					Serial.println("serve a");
					auto_.fa -= 1;
				}
				delay(time_to_eat);
				if (auto_.fb > 0)
				{
					serveB();
					//spinning_tray();
					Serial.println("serve b");
					auto_.fb -= 1;

				}
				delay(time_to_eat);
				if (auto_.fc > 0)
				{
					serveC();
					//spinning_tray();
					Serial.println("serve c");
					auto_.fc -= 1;
				}
				delay(time_to_eat);
				auto_.max_ -= 1;
			}
			get_mess = false;
			Serial.println("finish auto");
		}
	}
}

void loop()
{
	//auto_();
	
	// Home() is disable for testing

	if (((set() == 0) && (rotate() == 0)) || ((scoop() == 0) && (rotate() == 0)) || ((set() == 0) && (scoop() == 0))|| (set() == 0) && (rotate() == 0) && (scoop() == 0))
	{
		delay(100);
		auto_();
		if ((set() == 0) && (rotate() == 0) && (scoop() == 0))
		{
			notify();
			auto_();
			//delay(1000);
		}
	}
	else {

		auto_();
		if ((set() == 0) && (rotate() == 1) && (scoop() == 1))
		{
			//delay(10);
			if (set() == 0)
			{
				set_pos();
				//Serial.println(current_arm_pos);
				//Serial.println(current_spoon_pos);
			}
		}
		else if ((rotate() == 0) && (set() == 1) && ((scoop() == 1)))
		{
			//delay(5);
			
			if (rotate() == 0)
			{
				spinning_tray();
				Serial.println("rotate");
			}
		}
		else if (current_arm_pos > arm_home_pos)
		{
			if ((scoop() == 0) && (rotate() == 1) && (set() == 1))
			{
				//delay(2);
				if (scoop() == 0)
				{
					Serial.println("scooping_");
					scooping();

				}
			}
		}

	}
	

}