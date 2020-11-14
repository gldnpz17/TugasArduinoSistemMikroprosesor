#include <LiquidCrystal.h>
//Firdaus Bisma Suryakusuma
//19/444051/TK/49247
//Tugas Arduino Sistem Mikroprosesor

//UTILITIES
//a simple button press event handler because
//managing button states gets messy really fast.
typedef void (* EventHandler)();
class ButtonEvent
{
  private:
  int pinNumber;
  EventHandler eventHandler;
  int currentButtonState;
  bool buttonDown = false;
  
  public:
  ButtonEvent(int pinNumber, EventHandler eventHandler)
  {
    this->pinNumber = pinNumber;
    this->eventHandler = eventHandler;
  }
  
  void listen()
  {
    currentButtonState = digitalRead(pinNumber);
    
    //state transition to unpressed
    if (currentButtonState == LOW && buttonDown)
    {
      buttonDown = false;
    }
    //state transition to pressed
    else if (currentButtonState == HIGH && !buttonDown)
    {
      buttonDown = true;
      eventHandler();
    }
  }
};

//A simple event-driven timer
class Timer
{
  private:
  bool isActive = false;
  unsigned long interval = 0;
  EventHandler eventHandler;
  unsigned long lastTriggered = 0;
    
  public:
  Timer(int interval, EventHandler eventHandler)
  {
    this->interval = interval;
    this->eventHandler = eventHandler;
  }
  void track()
  {
    if (isActive)
    {
      if (millis() >= (lastTriggered + interval))
      {
        lastTriggered = millis();
        eventHandler();
      }
    }
  }
  void start()
  {
    isActive = true;
  }
  void stop()
  {
    isActive = false;
  }
};

long parseBinary(String binaryString)
{
  long output = 0;
  
  for(int x = 0; x < binaryString.length(); x++)
  {
    if(binaryString[x] == '1')
    {
      output += power(2, binaryString.length()-(x+1));
    }
  }
  
  return output;
}

long power(long base, long exponent)
{
  long output = 1;
  for(int x = 0; x < exponent; x++)
  {
    output *= base;
  }
  return output;
}

//CONSTANTS
const long INT_16_MAX_VALUE = power(2,16) - 1;

//buttons
const int ZERO_BUTTON = 2;
const int ONE_BUTTON = 3;
const int RESET_BUTTON = 4;
const int INCREMENT_BUTTON = 5;

//lcd
const int RS = 13;
const int EN = 12;
const int D4 = 11;
const int D5 = 10;
const int D6 = 9;
const int D7 = 8;

ButtonEvent zeroEvent = ButtonEvent(ZERO_BUTTON, (EventHandler)ZeroHandler);
ButtonEvent oneEvent = ButtonEvent(ONE_BUTTON, (EventHandler)OneHandler);
ButtonEvent resetEvent = ButtonEvent(RESET_BUTTON, (EventHandler)ResetHandler);
ButtonEvent incrementEvent = ButtonEvent(INCREMENT_BUTTON, (EventHandler)IncrementHandler);

Timer incrementTimer = Timer(10, (EventHandler)IncrementTimerHandler);

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

//APPLICATION STATE
String inputString = "";
long firstNumber = -1;
long secondNumber = -1;
bool canIncrement = false;

//APPLICATION FUNCTIONS
void ZeroHandler()
{
  EnterBit('0');
}

void OneHandler()
{
  EnterBit('1');
}

void ResetHandler()
{
  inputString = "";
  canIncrement = false;
  incrementTimer.stop();
  firstNumber = -1;
  secondNumber = -1;
  
  Serial.println("");
  Serial.println("[RESET]");
  
  lcd.clear();
  lcd.print("Input A(16-bit):");
}

void IncrementHandler()
{
  if (canIncrement)
  {
    incrementTimer.start();
  }
}

void IncrementTimerHandler()
{
  if (secondNumber < INT_16_MAX_VALUE)
  {
    secondNumber++;
    Serial.print("B = ");
    Serial.println(secondNumber, BIN);
  }
  if(secondNumber == INT_16_MAX_VALUE)
  {
    Serial.println("B sudah mencapai nilai maksimum.");
    canIncrement = false;
    incrementTimer.stop();
    return;
  }
  
}

void EnterBit(char character)
{
  lcd.setCursor(0,1);
  inputString += character;
  lcd.print(inputString);
  
  if(inputString.length() >= 16)
  {
    if (firstNumber == -1)
    {
      firstNumber = parseBinary(inputString);
      inputString = "";
      lcd.clear();
      lcd.print("Input B(16-bit):");
    }
    else if (secondNumber == -1)
    {
      secondNumber = parseBinary(inputString);
      inputString = "";
      
      if (IsEqual(firstNumber, secondNumber))
      {
        lcd.clear();
        lcd.print("A dan B Sama");
        Serial.println("A dan B Sama");
      }
      else
      {
        lcd.clear();
        lcd.print("A dan B Beda");
        Serial.println("A dan B Beda");

        if (firstNumber > secondNumber)
        {
          Serial.println("A > B. Press \"start increment\" to start incrementing.");
          canIncrement = true;
        }
        else
        {
          Serial.println("A < B. Jika B diincrement tidak akan pernah sama dengan A.");
        }
      }
      lcd.setCursor(0,1);
      lcd.print("Cek Serial Mon.");
    }
  }
}
bool IsEqual(int x, int y)
{
  return (x == y);
}

//SETUP AND LOOP
void setup()
{
  pinMode(ZERO_BUTTON, INPUT);
  pinMode(ONE_BUTTON, INPUT);
  pinMode(RESET_BUTTON, INPUT);
  pinMode(INCREMENT_BUTTON, INPUT);
  
  lcd.begin(16, 2);
  lcd.print("Input A(16-bit):");
  
  Serial.begin(9600);
}

void loop()
{
  zeroEvent.listen();
  oneEvent.listen();
  resetEvent.listen();
  incrementEvent.listen();
  
  incrementTimer.track();
}