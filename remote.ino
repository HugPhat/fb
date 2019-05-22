// first run: remote doesnt allow fb to scoop just rotate, need a var for first time setting

#define set_ A0
#define rot_ A3
#define sco_ A1


const int data_pin = 12;
const int transfer_speed = 1024;
const int btn_sco = 9;
const int btn_rot = 7;
const int buzzer_ = 4;
const int tone_ = 1500;
const int tone_1 = 1300;
bool set_up = false;  // First time setting

void setup() 
{
  pinMode(set_, OUTPUT);
  pinMode(rot_, OUTPUT);
  pinMode(sco_, OUTPUT);
  //digitalWrite(set_, HIGH);
  //digitalWrite(set_, HIGH);
  pinMode(btn_sco, INPUT);
  pinMode(btn_rot, INPUT);
  pinMode(buzzer_, OUTPUT);
  Serial.begin(9600);// giao tiếp Serial với baudrate 9600
  Serial.println("Ready.........");
}
void Off_set()
{
  pinMode(set_, OUTPUT);Serial.println("set on");
}
void On_set()
{
  pinMode(set_, INPUT);Serial.println("set off");
}
void Off_rot()
{
  pinMode(rot_, OUTPUT);Serial.println("rot on");
}
void On_rot()
{
  pinMode(rot_, INPUT);Serial.println("rot off");
}
void Off_sco()
{
  pinMode(sco_, OUTPUT);Serial.println("sco on");
  
}
void On_sco()
{
  pinMode(sco_, INPUT);Serial.println("sco off");
}
int check_btn_sco()
{
    int check_ = digitalRead(btn_sco);
    if (check_ == 1) return 1;
    else if (check_ == 0) return 0;
}
int check_btn_rot()
{
    int check_ = digitalRead(btn_rot);
    if (check_ == 1) return 1;
    else if (check_ == 0) return 0;
}
void buzzer(int times)
  {
    for(int i=0; i< times;i++)
    {
      tone(buzzer_, tone_);
      delay(200);
      noTone(buzzer_);
      delay(200);
      tone(buzzer_, tone_1);
      delay(500);
      noTone(buzzer_);
      delay(500);
    }
  }
void test_sound()
  {
    for(int i=100; i< 2000;i= i+200)
    {
      tone(buzzer_, i);
      delay(1000);
      Serial.println(i);
      noTone(buzzer_);
      delay(100);
    }
  }
void start_off()
{
    buzzer(2);
}
void loop() 
{
   if(set_up == false)
   {
     int wait_ = 0;
      while(check_btn_sco() == 0 && check_btn_rot() == 0)
      {
         wait_++;
         delay(2);
         if(wait_ > 20)
         {
            break;
         }
      }
      // nếu vẫn còn nhấn 
       if (check_btn_sco() == 0 && check_btn_rot() == 0)  buzzer(1); 
       while(check_btn_sco() == 0 && check_btn_rot() == 0)
        {
          On_set();
          set_up = true;  // init  
          
        }
      if(set_up == true){
        Off_set();
      }
          
   }
      while(set_up == true)
      {
        //Serial.println(set_up);
        if (check_btn_sco() == 0 && check_btn_rot() == 1)   // is scoop button pressed?
        {
          delay(30);
          if (check_btn_sco() == 0 && check_btn_rot() == 1)  
        {
          tone(buzzer_, tone_);
          delay(200);
          noTone(buzzer_);
          On_sco();
          On_sco();
          delay(1500);
          Off_sco();
        }
        }
        else if (check_btn_sco() == 1 && check_btn_rot() == 0)  // rotate button is pressed?
        {
          delay(40);
          if (check_btn_sco() == 1 && check_btn_rot() == 0)  
        {
          tone(buzzer_, tone_);
          delay(200);
          noTone(buzzer_);
          On_rot();
          On_rot();
          delay(1500);
          Off_rot();
        }
        }
        else if (check_btn_sco() == 0 && check_btn_rot() == 0) // is 2 buttons pressed?
        {
          delay(20);                                        // debounce
          while(check_btn_sco() == 0 && check_btn_rot() == 0) // exit, going to setting position
          {                    
            On_set();
          }
         for (int i = 0; i< 2; i++) // avoid the debounce from  RF module
            {    
             Off_set();
            } 
        }
      }
}
