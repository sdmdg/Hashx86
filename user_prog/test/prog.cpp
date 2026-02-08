/**
 * @file        prog.cpp
 * @brief       Calculator [BIN]
 *
 * @date        01/02/2026
 * @version     1.0.0
 */

#include <Hx86/Hgui/Hgui.h>
#include <Hx86/Hx86.h>
#include <Hx86/debug.h>
#include <prog.h>

Label* l1;

#define TOP_PADDING 120
#define LEFT_PADDING 10

int atoi(const char* str) {
    int res = 0;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res * sign;
}

double atof(const char* str) {
    double result = 0.0;
    double sign = 1.0;

    // Handle negative numbers
    if (*str == '-') {
        sign = -1.0;
        str++;
    }

    // Parse integer part
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }

    // Parse decimal part
    if (*str == '.') {
        str++;
        double fraction = 0.1;
        while (*str >= '0' && *str <= '9') {
            result += (*str - '0') * fraction;
            fraction *= 0.1;
            str++;
        }
    }

    return result * sign;
}

void itoa(int value, char* str) {
    char* p = str;
    bool isNegative = false;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0) {
        isNegative = true;
        value = -value;
    }

    // Convert digits in reverse order
    char* start = p;
    while (value > 0) {
        *p++ = '0' + (value % 10);
        value /= 10;
    }

    if (isNegative) *p++ = '-';
    *p = '\0';

    // Reverse string
    char* end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}

void ftoa(double value, char* str, int precision = 6) {
    // Handle negative numbers
    if (value < 0) {
        *str++ = '-';
        value = -value;
    }

    // Extract integer part
    int intPart = (int)value;
    double fractPart = value - intPart;

    // Convert integer part
    itoa(intPart, str);

    // Find end of string
    while (*str) str++;

    // Check if we need decimal part
    if (fractPart > 0.000001 && precision > 0) {
        *str++ = '.';

        // Convert fractional part
        for (int i = 0; i < precision; i++) {
            fractPart *= 10;
            int digit = (int)fractPart;
            *str++ = '0' + digit;
            fractPart -= digit;

            // Stop if we've reached the end of significant digits
            if (fractPart < 0.000001) break;
        }
    }

    *str = '\0';
}

Calculator::Calculator() {
    mainWindow = new Window(desktop, 210, 340, 210, 280);
    mainWindow->setWindowTitle("Calculator 1.0.0");
    screen = new Label(mainWindow, 10, 20, 190, 70, "0");
    screen->setSize(LARGE);

    // Add clear button at the top
    btn_clear = new Button(mainWindow, LEFT_PADDING + 3 * 50, (TOP_PADDING - 40), 40, 30, "C");

    btn_0 = new Button(mainWindow, LEFT_PADDING + 0 * 50, (TOP_PADDING + 3 * 40), 40, 30, "0");
    btn_dot = new Button(mainWindow, LEFT_PADDING + 1 * 50, (TOP_PADDING + 3 * 40), 40, 30, ".");
    btn_plus = new Button(mainWindow, LEFT_PADDING + 2 * 50, (TOP_PADDING + 3 * 40), 40, 30, "+");
    btn_solve = new Button(mainWindow, LEFT_PADDING + 3 * 50, (TOP_PADDING + 3 * 40), 40, 30, "=");

    btn_1 = new Button(mainWindow, LEFT_PADDING + 0 * 50, (TOP_PADDING + 2 * 40), 40, 30, "1");
    btn_2 = new Button(mainWindow, LEFT_PADDING + 1 * 50, (TOP_PADDING + 2 * 40), 40, 30, "2");
    btn_3 = new Button(mainWindow, LEFT_PADDING + 2 * 50, (TOP_PADDING + 2 * 40), 40, 30, "3");
    btn_minus = new Button(mainWindow, LEFT_PADDING + 3 * 50, (TOP_PADDING + 2 * 40), 40, 30, "-");

    btn_4 = new Button(mainWindow, LEFT_PADDING + 0 * 50, (TOP_PADDING + 1 * 40), 40, 30, "4");
    btn_5 = new Button(mainWindow, LEFT_PADDING + 1 * 50, (TOP_PADDING + 1 * 40), 40, 30, "5");
    btn_6 = new Button(mainWindow, LEFT_PADDING + 2 * 50, (TOP_PADDING + 1 * 40), 40, 30, "6");
    btn_multiplication =
        new Button(mainWindow, LEFT_PADDING + 3 * 50, (TOP_PADDING + 1 * 40), 40, 30, "*");

    btn_7 = new Button(mainWindow, LEFT_PADDING + 0 * 50, (TOP_PADDING + 0 * 40), 40, 30, "7");
    btn_8 = new Button(mainWindow, LEFT_PADDING + 1 * 50, (TOP_PADDING + 0 * 40), 40, 30, "8");
    btn_9 = new Button(mainWindow, LEFT_PADDING + 2 * 50, (TOP_PADDING + 0 * 40), 40, 30, "9");
    btn_division =
        new Button(mainWindow, LEFT_PADDING + 3 * 50, (TOP_PADDING + 0 * 40), 40, 30, "/");

    mainWindow->AddChild(screen);
    mainWindow->AddChild(btn_clear);
    mainWindow->AddChild(btn_0);
    mainWindow->AddChild(btn_1);
    mainWindow->AddChild(btn_2);
    mainWindow->AddChild(btn_3);
    mainWindow->AddChild(btn_4);
    mainWindow->AddChild(btn_5);
    mainWindow->AddChild(btn_6);
    mainWindow->AddChild(btn_7);
    mainWindow->AddChild(btn_8);
    mainWindow->AddChild(btn_9);
    mainWindow->AddChild(btn_dot);
    mainWindow->AddChild(btn_plus);
    mainWindow->AddChild(btn_solve);
    mainWindow->AddChild(btn_minus);
    mainWindow->AddChild(btn_multiplication);
    mainWindow->AddChild(btn_division);

    btn_0->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(0); });
    btn_1->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(1); });
    btn_2->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(2); });
    btn_3->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(3); });
    btn_4->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(4); });
    btn_5->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(5); });
    btn_6->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(6); });
    btn_7->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(7); });
    btn_8->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(8); });
    btn_9->OnClick(this, [](void* instance) { static_cast<Calculator*>(instance)->onPressNum(9); });

    btn_dot->OnClick(this,
                     [](void* instance) { static_cast<Calculator*>(instance)->onPressFunc('.'); });
    btn_plus->OnClick(this,
                      [](void* instance) { static_cast<Calculator*>(instance)->onPressFunc('+'); });
    btn_solve->OnClick(
        this, [](void* instance) { static_cast<Calculator*>(instance)->onPressFunc('='); });
    btn_minus->OnClick(
        this, [](void* instance) { static_cast<Calculator*>(instance)->onPressFunc('-'); });
    btn_multiplication->OnClick(
        this, [](void* instance) { static_cast<Calculator*>(instance)->onPressFunc('*'); });
    btn_division->OnClick(
        this, [](void* instance) { static_cast<Calculator*>(instance)->onPressFunc('/'); });

    btn_clear->OnClick(
        this, [](void* instance) { static_cast<Calculator*>(instance)->clearCalculator(); });

    mainWindow->show();
}

Calculator::~Calculator() {
    delete mainWindow;
}

void Calculator::onPressNum(uint32_t num) {
    if (newInput) {
        inputIndex = 0;
        input[0] = '\0';
        newInput = false;
        hasDecimal = false;
    }

    if (inputIndex < sizeof(input) - 2) {
        input[inputIndex++] = '0' + num;
        input[inputIndex] = '\0';
        screen->setText(input);
    }
    // printf("Pressed: %d\n", num);
}

void Calculator::onPressFunc(char func) {
    // Handle decimal point
    if (func == '.') {
        if (!hasDecimal && !newInput && inputIndex < sizeof(input) - 2) {
            // If input is empty, add a leading zero
            if (inputIndex == 0) {
                input[inputIndex++] = '0';
            }

            input[inputIndex++] = '.';
            input[inputIndex] = '\0';
            hasDecimal = true;
            screen->setText(input);
        }
        return;
    }

    // If user presses an operator or equals, first need to evaluate any pending operation
    if (input[0] != '\0' || func == '=' || hasResult) {
        double inputValue = (input[0] != '\0') ? atof(input) : currentValue;

        if (lastOperator == 0) {
            // First operation, just store the value
            currentValue = inputValue;
        } else {
            // Evaluate the pending operation
            switch (lastOperator) {
                case '+':
                    currentValue += inputValue;
                    break;
                case '-':
                    currentValue -= inputValue;
                    break;
                case '*':
                    currentValue *= inputValue;
                    break;
                case '/':
                    if (inputValue != 0)
                        currentValue /= inputValue;
                    else {
                        screen->setText("Div0 Err");
                        lastOperator = 0;
                        newInput = true;
                        return;
                    }

                    break;
            }
        }

        // Display the result
        char result[64];
        ftoa(currentValue, result);
        screen->setText(result);

        // Set flag that we have a result now
        hasResult = true;
    }

    // Set up for next input
    lastOperator = (func == '=') ? 0 : func;
    newInput = true;
}

void Calculator::evaluate() {
    onPressFunc('=');
}

void Calculator::clearCalculator() {
    // Reset all calculator state
    inputIndex = 0;
    input[0] = '\0';
    currentValue = 0;
    lastOperator = 0;
    newInput = true;
    hasDecimal = false;
    hasResult = false;

    // Reset display
    screen->setText("0");

    printf("Calculator cleared\n");
}

uint8_t inb(uint16_t portNumber) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(portNumber));
    return result;
}

void outb(uint16_t portNumber, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(portNumber));
}

void writeSerial(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0);  // Wait for the transmit buffer to be empty
    outb(0x3F8, c);
}

extern "C" void _start(void* arg) {
    init_sys(arg);
    init_graphics();

    printf("[Calculator]\n");

    // Test: try to read a hardware port from Ring 3 (should make GP)
    /*     printf("[Calculator] Attempting inb(0x3F8) from user mode...\n");
        uint8_t val = inb(0x3F8);
        printf("[Calculator] inb(0x3F8) = 0x%x\n", (uint32_t)val); */

    Calculator* cl = new Calculator();
}
