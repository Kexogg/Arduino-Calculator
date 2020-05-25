/*
Arduino calculator v 1.1
By Alexander Mazhirin
My e-mail: kexogg@gmail.com
Keyboard keys:
S - sin; O - cos; T - tan; I - PI; R - Square root; F - Factorial; B - Backspace; [ - Switch to first number;
] - Switch to second number; L - Toggle backlight; N - Switch numbers; C - Clear; P - Calculate with percentages
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
// Settings
#define LCD_Address 0x3f
#define LCD_Power 13
#define ROWS 5
#define COLS 6
#define rowPins \
    new byte[ROWS] { 6, 7, 8, 9, 10 }
#define colPins \
    new byte[COLS] { 5, 4, 3, 2, 0, 1 }
char keys[ROWS][COLS] = {
    {'1', '2', '3', '+', '^', 'S'},
    {'4', '5', '6', '-', 'R', 'O'},
    {'7', '8', '9', '*', 'F', 'T'},
    {'0', 'P', 'C', '/', 'B', 'I'},
    {'=', '.', '[', ']', 'L', 'N'}};
// Code
Keypad pad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(LCD_Address, 20, 4);
boolean ModifyingFirstNumber = true; //Are we modifying first number
boolean IsBacklightON = true;        //Is backlight on
boolean PercentFlag = false;         //Are percentages used
boolean gotResult = false;           //Is result displayed
String FirstNumber = "";
String SecondNumber = "";
String tmp = "";
double result = 0;
char action = ' ';

void setup()
{
    pinMode(LCD_Power, OUTPUT); //Turn LCD on
    digitalWrite(LCD_Power, HIGH);
    lcd.init();      //Initialise LCD
    lcd.backlight(); //Turn LCD backlight on
    redraw();        //Draw UI
}

void loop()
{
    char Key = pad.getKey(); //Get key
    if (Key != NO_KEY)       //If key pressed
    {
        if (isDigit(Key) && !gotResult) //If key is a digit
        {
            if (ModifyingFirstNumber) //Check which number to modify
            {
                if (FirstNumber.length() < 19) //Don't add new digit if number can't fit on display
                {
                    FirstNumber = FirstNumber + Key; //Modify number
                    lcd.print(Key);                  //Print new digit
                }
            }
            else
            {
                if (SecondNumber.length() < 19) //Don't add new digit if number can't fit on display
                {
                    SecondNumber = SecondNumber + Key; //Modify number
                    lcd.print(Key);                    //Print new digit
                }
            }
        }
        if (String("+-*/^").indexOf(Key) != -1) //If key is operator
        {
            gotResult = false;
            PercentFlag = false;
            if (!ModifyingFirstNumber) //If modifying second number
            {                          //Change operator
                action = Key;
                redraw();
            }
            else //Change operator and switch to second number
            {
                ModifyingFirstNumber = false;
                action = Key;
                redraw();
            }
        }
        if (String("RSOTF").indexOf(Key) != -1 && FirstNumber != "") //If key is operator that doesn't require second number
        {
            gotResult = false;
            action = Key;
            calculate(); //Skip second number and calculate result
        }
        switch (Key) //Special cases
        {
        case '=': //Calculate result
            if (action != ' ' && !gotResult) //If operator set and result is not displayed
            {
                calculate();
            }
            break;
        case 'C': //Clear display and data
            clear();
            break;
        case 'B': //Remove last symbol
            if (!gotResult) //If result is not displayed
            {
                if (ModifyingFirstNumber) //Check which number to modify
                {
                    FirstNumber.remove(FirstNumber.length() - 1); //Remove last symbol
                    redraw();                                     //Update display
                }
                else
                {
                    SecondNumber.remove(SecondNumber.length() - 1); //Remove last symbol
                    redraw();                                       //Update display
                }
            }
            break;
        case '.': // Add point to current number
            if (ModifyingFirstNumber) //Check which number to modify
            {
                if (FirstNumber.length() < 18 && (FirstNumber.indexOf('.') == -1)) //If point isn't present in number
                {                                                                  // and length of number is < 18
                    FirstNumber = FirstNumber + Key;                               //Add point
                    lcd.print(Key);                                                //Print point
                }
            }
            else
            {
                if (SecondNumber.length() < 18 && (SecondNumber.indexOf('.') == -1)) //If point isn't present in number
                {                                                                    // and length of number is < 18
                    SecondNumber = SecondNumber + Key;                               //Add point
                    lcd.print(Key);                                                  //Print point
                }
            }
            break;
        case 'P': //Use percentages
            calculatePercentages(); //Calculate result with percentages
            break;
        case '[': //Switch to first number
            if (!gotResult && !ModifyingFirstNumber) //If result isn't displayed and not modifying first number
            {
                ModifyingFirstNumber = true;       //Switch to first number
                DrawMessage("Editing 1st number"); //Display message
            }
            break;
        case ']': //Switch to second number
            if (!gotResult && ModifyingFirstNumber) //If result isn't displayed and modifying first number
            {
                ModifyingFirstNumber = false;      //Switch to second number
                DrawMessage("Editing 2nd number"); //Display message
            }
            break;
        case 'L': //Toggle backlight
            if (IsBacklightON) //If backlight is ON
            {
                lcd.noBacklight(); //Turn backlight off
                IsBacklightON = false;
                DrawMessage("Backlight OFF"); //Display message
            }
            else
            {
                lcd.backlight(); //Turn backlight on
                IsBacklightON = true;
                DrawMessage("Backlight ON"); //Display message
            }
            break;
        case 'N': //Switch numbers
            if (!gotResult) //If result isn't displayed
            {
                tmp = FirstNumber; //Switch numbers
                FirstNumber = SecondNumber;
                SecondNumber = tmp;
                redraw();
                DrawMessage("Switched numbers"); //Display message
                tmp = "";
            }
            break;
        case 'I': //Use PI as current number
            if (!gotResult) //If result isn't displayed
            {
                if (ModifyingFirstNumber) //Check which number to modify
                {
                    FirstNumber = tmp + PI; //Print PI
                }
                else
                {
                    SecondNumber = tmp + PI; //Print PI
                }
            }
        default:
            break;
        }
    }
}

void calculate() //Calculate result
{
    switch (action)
    {
    case '+':
        result = FirstNumber.toFloat() + SecondNumber.toFloat();
        drawResult();
        break;
    case '-':
        result = FirstNumber.toFloat() - SecondNumber.toFloat();
        drawResult();
        break;
    case '*':
        result = FirstNumber.toFloat() * SecondNumber.toFloat();
        drawResult();
        break;
    case '/':
        if (SecondNumber.toFloat() == 0) //If second number equals 0
        {
            DrawMessage("ERROR: Division by 0"); //Display error message
        }
        else
        {
            result = FirstNumber.toFloat() / SecondNumber.toFloat();
            drawResult();
        }
        break;
    case '^':
        result = pow(FirstNumber.toFloat(), SecondNumber.toFloat());
        drawResult();
        break;
    case 'R': //sqrt
        result = sqrtf(FirstNumber.toFloat());
        drawSNResult();
        break;
    case 'S': //sin
        result = sin(radians(FirstNumber.toFloat()));
        drawSNResult();
        break;
    case 'O': //cos
        result = cos(radians(FirstNumber.toFloat()));
        drawSNResult();
        break;
    case 'T': //tg
        result = tan(radians(FirstNumber.toFloat()));
        drawSNResult();
        break;
    case 'F':
        if (FirstNumber.toInt() < 15 && FirstNumber.toInt() > -15) //Arduino can't calculate factorial for numbers larger than 14
        {                                                          //or less than -14
            calculateFactorial();
        }
        else
        {
            DrawMessage("ERROR: Overflow"); //Display error message if number is <-14 or >14
        }
        break;
    default:
        break;
    }
}

void calculatePercentages() //Calculate result for expression with percentages
{
    PercentFlag = true; //Set flag to draw '%'
    switch (action)     //Calculate and draw result
    {
    case '+':
        result = FirstNumber.toFloat() + ((FirstNumber.toFloat() / 100 * SecondNumber.toFloat()));
        drawResult();
        break;
    case '-':
        result = FirstNumber.toFloat() - ((FirstNumber.toFloat() / 100 * SecondNumber.toFloat()));
        drawResult();
        break;
    case '*':
        result = (FirstNumber.toFloat() / 100 * SecondNumber.toFloat());
        drawResult();
        break;
    case '/':
        result = (FirstNumber.toFloat() / 100 * SecondNumber.toFloat());
        drawResult();
        break;
    default:
        break;
    }
}

void calculateFactorial() //Calculate factorial
{
    long factorialresult = (FirstNumber.toInt());                                        //Declare variable with diffrent data type
    for (int factorialCount = 1; factorialCount < FirstNumber.toInt(); factorialCount++) //Calculate factorial
    {
        factorialresult = factorialresult * factorialCount; //n! = 1 * 2 * 3  * ... * n
    }
    lcd.clear(); //Display result
    lcd.print(FirstNumber);
    lcd.print('!');
    lcd.setCursor(0, 1);
    lcd.print('=');
    if (factorialresult == 0) //Exception: 0! = 1
    {
        lcd.print('1');
    }
    else
    {
        lcd.print(factorialresult);
    }
    gotResult = true;
    FirstNumber = String(factorialresult);
    SecondNumber = "";
}

void drawResult() //Display result
{
    lcd.clear();
    lcd.home();
    if ((FirstNumber.length()) + (SecondNumber.length()) + 1 <= 20) //If both numbers can fit on same lane
    {
        lcd.print(FirstNumber); //Draw layout for short numbers
        lcd.print(action);
        lcd.print(SecondNumber);
        if (PercentFlag)
        {
            lcd.print('%');
            PercentFlag = false;
        }
        lcd.setCursor(0, 1);
        lcd.print('=');
        lcd.print(result);
    }
    else
    {
        redraw(); //If not, draw layout for long numbers
        lcd.setCursor(0, 3);
        lcd.print('=');
        lcd.print(result);
    }
    gotResult = true;
    FirstNumber = String(result);
    SecondNumber = "";
}

void drawSNResult() //Display result for expression which requires only one number
{
    lcd.home();
    lcd.clear();    //Clear display
    switch (action) //Display operator
    {
    case 'R':
        lcd.print("sqrt(");
        break;
    case 'S':
        lcd.print("sin(");
        break;
    case 'O':
        lcd.print("cos(");
        break;
    case 'T':
        lcd.print("tg(");
        break;
    default:
        break;
    }
    lcd.print(FirstNumber); //Display first number
    lcd.print(')');
    lcd.setCursor(0, 1);
    lcd.print('=');
    lcd.print(result); //Display result
    gotResult = true;
    FirstNumber = String(result);
    SecondNumber = "";
}

void clear() //Reset data and update display
{
    lcd.clear(); //Display default layout
    lcd.setCursor(0, 1);
    lcd.print("Operation: ");
    lcd.setCursor(0, 0);
    FirstNumber = ""; //Reset data
    SecondNumber = "";
    result = 0;
    action = ' ';
    ModifyingFirstNumber = true;
    gotResult = false;
    PercentFlag = false;
    DrawMessage("Cleared"); //Display message
}

void redraw() //Update display (redraw)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(FirstNumber); //Display first number
    lcd.setCursor(0, 1);
    lcd.print("Operation: "); //Display operator
    lcd.print(action);
    lcd.setCursor(0, 2);
    lcd.print(SecondNumber); //Display second number
    if (PercentFlag)         //Display '%' if needed
    {
        lcd.print('%');
        PercentFlag = false;
    }
    if (ModifyingFirstNumber) //Set cursor to correct position
    {
        lcd.setCursor(FirstNumber.length(), 0);
    }
    else
    {
        lcd.setCursor(SecondNumber.length(), 2);
    }
}

int DrawMessage(String tmp) //Display message
{
    if (!gotResult) //If result isn't displayed
    {
        lcd.setCursor(0, 3);
        lcd.print(tmp); //Display message
        delay(500);     //FIXME: delay interrupting program
        redraw();       //Erase message
        tmp = "";
    }
}