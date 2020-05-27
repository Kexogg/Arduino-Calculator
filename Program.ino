/*
Arduino calculator v 1.3
By Alexander Mazhirin
My e-mail: kexogg@gmail.com
GitHub page: https://github.com/Kexogg/Arduino-Calculator
Keyboard keys:
S - sin; O - cos; T - tan; D - Switch between RAD and DEG; R - Square root; F - Factorial; B - Backspace; [ - Switch to first number;
] - Switch to second number; L - Toggle backlight; N - Switch numbers; C - Clear; P - Calculate with percentages
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
// Settings
#define LCD_Address 0x3f
#define ROWS 5
#define COLS 6
#define rowPins \
    new byte[ROWS] { 6, 7, 8, 9, 10 }
#define colPins \
    new byte[COLS] { 5, 4, 3, 2, 0, 1 }
char keys[ROWS][COLS] = {
    {'1', '2', '3', '+', '^', 'S'},
    {'4', '5', '6', '-', 'R', 'O'},
    {'7', '8', '9', '*', '!', 'T'},
    {'0', 'P', 'C', '/', 'B', 'D'},
    {'=', '.', '[', ']', 'L', 'N'}};
// Code
Keypad pad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(LCD_Address, 20, 4);
bool ModifyingFirstNumber = true; //Are we modifying first number
bool SafeToDrawMessage = true;    //Is it safe to draw message
bool IsBacklightON = true;        //Is backlight on
bool PercentFlag = false;         //Are percentages used
bool FactorialFlag = false;       //Is factorial used
bool gotResult = false;           //Is result displayed
bool useRAD = false;              //Are we using RAD
String FirstNumber = "";
String SecondNumber = "";
String tmp = "";
String SNE = "RSOT!"; //List of single number expressions
long factorialResult;
long resultLong;
int factorialCount;
double result = 0;
double trigonometryConvert = 0;
char action = ' ';

void setup()
{
    pinMode(13, OUTPUT); //Turn LCD on
    digitalWrite(13, HIGH);
    lcd.init();      //Initialise LCD
    lcd.backlight(); //Turn LCD backlight on
    redraw();        //Draw UI
}

void loop()
{
    char Key = pad.getKey(); //Get key
    if (Key != NO_KEY)       //If key is pressed
    {
        if (isDigit(Key) && !gotResult) //If key is a digit
        {
            if (ModifyingFirstNumber) //Check which number to modify
            {
                if (FirstNumber.length() <= 19) //Don't add new digit if number can't fit on display
                {
                    FirstNumber = FirstNumber + Key; //Modify number
                    lcd.print(Key);                  //Print new digit
                }
                else
                {
                    drawMessage("ERROR: Overflow");
                }
            }
            else
            {
                if (SecondNumber.length() <= 19) //Don't add new digit if number can't fit on display
                {
                    SecondNumber = SecondNumber + Key; //Modify number
                    lcd.print(Key);                    //Print new digit
                }
                else
                {
                    drawMessage("ERROR: Overflow");
                }
            }
        }
        if (String("+-*/^").indexOf(Key) != -1) //If key is operator
        {
            SafeToDrawMessage = true;
            FactorialFlag = false;
            PercentFlag = false;
            gotResult = false;
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
        if (SNE.indexOf(Key) != -1 && FirstNumber != "") //If key is operator that doesn't require second number
        {
            SecondNumber = "";
            SafeToDrawMessage = true;
            FactorialFlag = false;
            gotResult = false;
            action = Key;
            calculate(); //Skip second number and calculate result
        }
        if (!gotResult) //Special cases
        {
            switch (Key)
            {
            case '=':              //Calculate result
                if (action != ' ') //If operator set
                {
                    calculate();
                }
                else
                {
                    drawMessage("ERROR: No operation");
                }
                break;
            case 'B':                     //Remove last symbol
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
                break;
            case '.':                     // Add point to current number
                if (ModifyingFirstNumber) //Check which number to modify
                {
                    if (FirstNumber.length() < 18 && (FirstNumber.indexOf('.') == -1)) //If point isn't present in number
                    {                                                                  //and length of number is < 18
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
            case 'P':                   //Use percentages
                calculatePercentages(); //Calculate result with percentages
                break;
            case '[':                      //Switch to first number
                if (!ModifyingFirstNumber) //If modifying first number
                {
                    ModifyingFirstNumber = true;       //Switch to first number
                    drawMessage("Editing 1st number"); //Display message
                }
                break;
            case ']':                     //Switch to second number
                if (ModifyingFirstNumber) //If modifying first number
                {
                    ModifyingFirstNumber = false;      //Switch to second number
                    drawMessage("Editing 2nd number"); //Display message
                }
                break;
            case 'N': //Switch numbers
                tmp = FirstNumber;
                FirstNumber = SecondNumber;
                SecondNumber = tmp;
                redraw();
                drawMessage("Switched numbers"); //Display message
                tmp = "";
                break;
            }
        }
        switch (Key)
        {
        case 'L':              //Toggle backlight
            if (IsBacklightON) //If backlight is ON
            {
                lcd.noBacklight(); //Turn backlight off
                IsBacklightON = false;
                drawMessage("Backlight OFF"); //Display message
            }
            else
            {
                lcd.backlight(); //Turn backlight on
                IsBacklightON = true;
                drawMessage("Backlight ON"); //Display message
            }
            break;
        case 'C': //Clear display and data
            clear();
            break;
        case 'D': //Switch between RAD and DEG
            useRAD = !useRAD;
            trigonometryUpdate();
            break;
        }
    }
}

void calculate() //Calculate result
{
    switch (action)
    {
    case '+':
        result = FirstNumber.toDouble() + SecondNumber.toDouble();
        break;
    case '-':
        result = FirstNumber.toDouble() - SecondNumber.toDouble();
        break;
    case '*':
        result = FirstNumber.toDouble() * SecondNumber.toDouble();
        break;
    case '/':
        if (SecondNumber.toDouble() == 0) //If second number equals 0
        {
            drawMessage("ERROR: Division by 0"); //Display error message
        }
        else
        {
            result = FirstNumber.toDouble() / SecondNumber.toDouble();
            drawResultUI();
        }
        break;
    case '^':
        result = pow(FirstNumber.toDouble(), SecondNumber.toDouble());
        break;
    case 'R': //sqrt
        result = sqrtf(FirstNumber.toDouble());
        break;
    case 'S': //sin
        result = sin(trigonometrySwitch(FirstNumber.toDouble()));
        break;
    case 'O': //cos
        result = cos(trigonometrySwitch(FirstNumber.toDouble()));
        break;
    case 'T': //tan
        result = tan(trigonometrySwitch(FirstNumber.toDouble()));
        break;
    case '!':
        if (FirstNumber.toInt() < 15 && FirstNumber.toInt() > -15) //Arduino can't calculate factorial for
        {                                                          //numbers larger than 14 or less than -14
            calculateFactorial();
        }
        else
        {
            drawMessage("ERROR: Overflow"); //Display error message if number is <-14 or >14
        }
        break;
    }
    if (action != '!' && action != '/')
    {
        drawResultUI();
    }
}

void calculatePercentages() //Calculate result for expression with percentages
{
    PercentFlag = true; //Set flag to draw '%'
    switch (action)     //Calculate and draw result
    {
    case '+':
        result = FirstNumber.toDouble() + ((FirstNumber.toDouble() / 100 * SecondNumber.toDouble()));
        break;
    case '-':
        result = FirstNumber.toDouble() - ((FirstNumber.toDouble() / 100 * SecondNumber.toDouble()));
        break;
    case '*':
        result = (FirstNumber.toDouble() / 100 * SecondNumber.toDouble());
        break;
    case '/':
        result = (FirstNumber.toDouble() / 100 * SecondNumber.toDouble());
        break;
        drawResultUI();
    }
}

void calculateFactorial() //Calculate factorial
{
    factorialResult = (FirstNumber.toInt()); //Declare variable with diffrent data type
    if (factorialResult == 0)                //Exception: 0! = 1
    {
        factorialResult = 1;
    }
    else
    {
        for (factorialCount = 1; factorialCount < FirstNumber.toInt(); factorialCount++) //Calculate factorial
        {
            factorialResult = factorialResult * factorialCount; //n! = 1 * 2 * ... * n
        }
    }
    FactorialFlag = true;
    drawResultUI();
}

void drawResultUI()
{
    if (isnan(result) || isinf(result) || result > 4294967040 || result < -4294967040) //Print error message if number is invalid
    {
        drawMessage("ERROR: Overflow");
    }
    else if (SNE.indexOf(action) != -1)
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
            lcd.print("tan(");
            break;
        }
        lcd.print(stringConvert(FirstNumber)); //Display first number
        if (action == '!')
        {
            lcd.print(action);
        }
        else
        {
            lcd.print(')');
        }
        lcd.setCursor(0, 1);
        lcd.print('=');
        printResult(); //Display result
    }
    else
    {
        lcd.clear();
        lcd.home();
        if ((FirstNumber.length()) + (SecondNumber.length()) + 2 <= 17) //If both numbers can fit on same lane
        {
            lcd.print(stringConvert(FirstNumber)); //Draw layout for short numbers
            lcd.print(action);
            lcd.print(stringConvert(SecondNumber));
            if (PercentFlag)
            {
                lcd.print('%');
                PercentFlag = false;
            }
            lcd.setCursor(0, 1);
            lcd.print('=');
            printResult();
        }
        else
        {
            redraw(); //If not, draw layout for long numbers
            lcd.setCursor(0, 3);
            lcd.print('=');
            printResult();
            SafeToDrawMessage = false;
        }
    }
    trigonometryUpdate();
}

void clear() //Reset data and update display
{
    FirstNumber = ""; //Reset data
    SecondNumber = "";
    result = 0;
    action = ' ';
    ModifyingFirstNumber = true;
    FactorialFlag = false;
    gotResult = false;
    PercentFlag = false;
    redraw();
    drawMessage("Cleared"); //Display message
}

void redraw() //Update display (redraw default layout)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(FirstNumber); //Display first number
    lcd.setCursor(0, 1);
    lcd.print("Operation: "); //Display operator
    lcd.print(action);
    trigonometryUpdate();
    lcd.setCursor(0, 2);
    lcd.print(SecondNumber); //Display second number
    if (PercentFlag)         //Display '%' if needed
    {
        lcd.print('%');
        PercentFlag = false;
    }
    setCursor();
    SafeToDrawMessage = true;
}

void setCursor()
{
    if (ModifyingFirstNumber) //Set cursor to correct position
    {
        lcd.setCursor(FirstNumber.length(), 0);
    }
    else
    {
        lcd.setCursor(SecondNumber.length(), 2);
    }
}

void printResult() //Print result in long data type (if possible)
{
    resultLong = result;
    if (resultLong == result)
    {
        lcd.print(resultLong);
        FirstNumber = String(resultLong);
    }
    else
    {
        lcd.print(result);
        FirstNumber = String(result);
    }
    if (FactorialFlag)
    {
        lcd.print(long(factorialResult));
        FirstNumber = String(factorialResult);
    }
    SecondNumber = "";
    gotResult = true;
}

void trigonometryUpdate() //Update trigonometry settings on display
{
    lcd.setCursor(17, 1);
    if (useRAD)
    {
        lcd.print("RAD");
    }
    else
    {
        lcd.print("DEG");
    }
    setCursor();
}

int drawMessage(String tmp) //Display message
{
    if (SafeToDrawMessage) //If it's safe to disply message
    {
        lcd.setCursor(0, 3);
        lcd.print(tmp);      //Display message
        delay(500);          //FIXME: delay interrupting program
        lcd.setCursor(0, 3); //Erase message
        lcd.print("                    ");
        setCursor(); //Set cursor to correct position
        tmp = "";
    }
}

double trigonometrySwitch(double trigonometryConvert) //Switch between DEG and RAD
{
    if (useRAD) //If we want radians
    {
        return trigonometryConvert; //return radians
    }
    else //If we want degrees
    {
        return radians(trigonometryConvert); //convert to degrees
    }
}

String stringConvert(String tmp)
{
    if (tmp.toInt() == tmp.toDouble())
    {
        return String(tmp.toInt());
    }
    else
    {
        return String(tmp.toDouble());
    }
}
