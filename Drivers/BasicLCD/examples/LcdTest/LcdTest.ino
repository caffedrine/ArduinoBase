#include <Arduino.h>
#include <BasicLCD.h>

using namespace Drivers;

#define PIN_LCD_D4					24
#define PIN_LCD_D5					25
#define PIN_LCD_D6					26
#define PIN_LCD_D7					27
#define PIN_LCD_RS					22
#define PIN_LCD_EN					23

BasicLCD lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

void setup()
{
	Serial.begin(115200);
	lcd.Init(16, 2);

	lcd.PrintLine("  HELLO", 0);
	lcd.PrintLine("   WORLD", 1);
}

unsigned long i = 0, j = 0;

void loop()
{
	static unsigned long PrevMillisI = 0, PrevMillisJ = 0;

	if(millis() - PrevMillisI >= 1000)
	{
		PrevMillisI = millis();
		i++;
		lcd.PrintLine( String(i), 0);
	}

	if(millis() - PrevMillisJ >= 500)
	{
		PrevMillisJ = millis();
		j+=2;
		lcd.PrintLine( String(j), 1);
	}

	lcd.Update();
}
