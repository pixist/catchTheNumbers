#include "mbed.h"
#include "TextLCD.h"
#include "Keypad.h"
#include "ReceiverIR.h"
#include <stdio.h>
#include <stdlib.h>
#define TEST_LOOP_BACK  0



TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
Keypad kpad(PTA12, PTD4, PTA2, PTA1, PTC9, PTC8, PTA5, PTA4);
PwmOut ledGreen(LED_GREEN);
PwmOut ledRed(LED_RED);
PwmOut ledBlue(LED_BLUE);
Ticker ledTicker;
ReceiverIR ir_rx(PTD6);
SPI spi(PTD2, PTD3, PTD1);          // Arduino compatible MOSI, MISO, SCLK
DigitalOut cs(PTD7);
Timer timer;
int recentIndex;
int round = 0;
int scoreTotal = 0;

const unsigned char receiveCodes[] { 230, 186, 185, 184, 187, 191, 188, 248, 234, 246, 233, 242, 231, 247, 227, 165, 173, 0};
const char* const receiveCodesEq[] { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "#", "up", "left", "ok", "right", "down", "?"};
const char* const levelNames[] {"Tutorial", "Easy", "Simple", "Cake", "Normal", "Hard", "Harder", "Bonkers", "Impossible","mAcHiNe ElVeS"};


const unsigned char ledNum[18][8] = {
    {0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E, 0x00},
    {0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00, 0x00},
    {0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00, 0x00},
    {0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00},
    {0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x00},
    {0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00, 0x00},
    {0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00, 0x00},
    {0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00, 0x00},
    {0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00},
    {0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00, 0x00},
    {0x08, 0x2A, 0x3E, 0x1C, 0x1C, 0x3E, 0x2A, 0x08},
    {0x14, 0x7F, 0x7F, 0x14, 0x7F, 0x7F, 0x14, 0x00},
    {0x0C, 0x66, 0xEC, 0xE0, 0xE0, 0xEC, 0x66, 0x0C},
    {0x04, 0x0E, 0x2A, 0x40, 0x40, 0x24, 0x0E, 0x0A},
    {0x0B, 0x15, 0x48, 0xA0, 0xA0, 0x48, 0x15, 0x0B},
    {0x0A, 0x0E, 0x24, 0x40, 0x40, 0x2A, 0x0E, 0x04},
    {0x0C, 0x18, 0x4C, 0x20, 0x20, 0x4C, 0x18, 0x0C},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    /*
    {0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00},
    {0x08, 0x1C, 0x36, 0x22, 0x08, 0x1C, 0x36, 0x22},
    {0x00, 0x00, 0x06, 0x5F, 0x5F, 0x06, 0x00, 0x00},
    {0x22, 0x36, 0x1C, 0x08, 0x22, 0x36, 0x1C, 0x08},
    {0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00}
    */
    };

/// Send two bytes to SPI bus
void SPI_Write2(unsigned char MSB, unsigned char LSB)
{
    cs = 0;                         // Set CS Low
    spi.write(MSB);                 // Send two bytes
    spi.write(LSB);
    cs = 1;                         // Set CS High
}
 
/// MAX7219 initialisation
void Init_MAX7219(void)
{
    SPI_Write2(0x09, 0x00);         // Decoding off
    SPI_Write2(0x0A, 0x08);         // Brightness to intermediate
    SPI_Write2(0x0B, 0x07);         // Scan limit = 7
    SPI_Write2(0x0C, 0x01);         // Normal operation mode
    SPI_Write2(0x0F, 0x0F);         // Enable display test
    wait_ms(500);                   // 500 ms delay
    SPI_Write2(0x01, 0x00);         // Clear row 0.
    SPI_Write2(0x02, 0x00);         // Clear row 1.
    SPI_Write2(0x03, 0x00);         // Clear row 2.
    SPI_Write2(0x04, 0x00);         // Clear row 3.
    SPI_Write2(0x05, 0x00);         // Clear row 4.
    SPI_Write2(0x06, 0x00);         // Clear row 5.
    SPI_Write2(0x07, 0x00);         // Clear row 6.
    SPI_Write2(0x08, 0x00);         // Clear row 7.
    SPI_Write2(0x0F, 0x00);         // Disable display test
    wait_ms(500);                   // 500 ms delay
}


/**
 * Receive.
 *
 * @param format Pointer to a format.
 * @param buf Pointer to a buffer.
 * @param bufsiz Size of the buffer.
 *
 * @return Bit length of the received data.
 */
int receive(RemoteIR::Format *format, uint8_t *buf, int bufsiz, int timeout = 100) {
    int cnt = 0;
    while (ir_rx.getState() != ReceiverIR::Received) {
        cnt++;
        if (timeout < cnt) {
            return -1;
        }
    }
    return ir_rx.getData(format, buf, bufsiz * 8);
}



/**
 * Display a current status.
 */
void display_status(char *status, int bitlength) {
    lcd.locate(8, 0);
    lcd.printf("%-5.5s:%02d", status, bitlength);
}

/**
 * Display a format of a data.
 */
void display_format(RemoteIR::Format format) {
    lcd.locate(0, 0);
    switch (format) {
        case RemoteIR::UNKNOWN:
            lcd.printf("????????");
            break;
        case RemoteIR::NEC:
            lcd.printf("NEC     ");
            break;
        case RemoteIR::NEC_REPEAT:
            lcd.printf("NEC  (R)");
            break;
        case RemoteIR::AEHA:
            lcd.printf("AEHA    ");
            break;
        case RemoteIR::AEHA_REPEAT:
            lcd.printf("AEHA (R)");
            break;
        case RemoteIR::SONY:
            lcd.printf("SONY    ");
            break;
    }
}

/**
 * Display a data.
 *
 * @param buf Pointer to a buffer.
 * @param bitlength Bit length of a data.
 */
void display_data(uint8_t *buf, int bitlength) {
    lcd.locate(0, 1);
    const int n = bitlength / 8 + (((bitlength % 8) != 0) ? 1 : 0);
    for (int i = 0; i < n; i++) {
        lcd.printf("%02X", buf[i]);
    }
    for (int i = 0; i < 8 - n; i++) {
        lcd.printf("--");
    }
}
/*
int old_get_data(uint8_t *buf, int bitlength) {
    const int n = bitlength / 8 + (((bitlength % 8) != 0) ? 1 : 0);
    char hexstring[n];
    for (int i = 0; i < n; i++) {
        hexstring[i] = buf[i];
    }
    return (int)strtol(hexstring, NULL, 16);
}
*/
int get_data(uint8_t *buf, int bitlength) {
    const int n = bitlength / 8 + (((bitlength % 8) != 0) ? 1 : 0);
    char hexstring[n];
    for (int i = 0; i < n; i++) {
        sprintf(hexstring, "%02X", buf[i]);
    }
    return (int)strtol(hexstring, NULL, 16);
}

void writeLED(int index)
{   
    if (index != recentIndex) 
    {
    for(int i=1; i < 9; i++)      // Write first character (8 rows)
        SPI_Write2(i,ledNum[index][i-1]);
    recentIndex = index;
    }
}

int IRReceive(int waitTime)
{   
    int index = 17;
    timer.reset();
    timer.start();
    while((index == 17)) {
        if ((timer.read_ms() > waitTime) && (waitTime > 0))
        {   
            timer.stop();
            index = -1;
            break;
        }
        uint8_t buf1[32];
        int bitlength1;
        int bufInt;
        int dataHolder;
        RemoteIR::Format format;
        memset(buf1, 0x00, sizeof(buf1));
        {
            bitlength1 = receive(&format, buf1, sizeof(buf1));
            if (bitlength1 < 0)
            {
                continue;
            }
            dataHolder = get_data(buf1, bitlength1);
            bufInt = dataHolder == 0 ? bufInt : dataHolder;
            for (index = 0; index < 18; index++)
            {
                if (bufInt == receiveCodes[index])
                {
                    break;
                }
            }
        }
    }
    timer.stop();
    return index;
}

void IRget(int desire)
{
    int ret;
    while (ret != desire)
    {
        ret = IRReceive(-1);
    }  
}


int main()
{
    cs = 1;                         // CS initially High
    spi.format(8,0);                // 8-bit format, mode 0,0
    spi.frequency(1000000);         // SCLK = 1 MHz
    Init_MAX7219(); 
    lcd.cls();
    lcd.printf("*CATCH THEM ALL*-by pixist");
    wait(3);
    lcd.cls();
    char key;
    int released = 1;
    char inputArray[2];
    int i = 0 ;
start:
    lcd.cls();
    lcd.printf("Choose a level: ");

    while((i<1) && (key != '#')){
        key = kpad.ReadKey();                   //read the current key pressed

        if(key == '\0')
            released = 1;                       //set the flag when all keys are released
           
        if((key != '\0') && (released == 1) && (key != '#')) {  //if a key is pressed AND previous key was released
            lcd.printf("*%c", key);
            inputArray[i] = key; 
            i++;         
            released = 0;                       //clear the flag to indicate that key is still pressed
        }
    }
    if (inputArray[0] == 'A')
    {   
        int index;
        int leaveCount = 0;
        lcd.cls();
        lcd.printf("Developer Mode A");
        wait(3);
        while (leaveCount < 4)
        {
            index = IRReceive(-1);
            if (index == 14) {leaveCount++;}
            else {leaveCount = 0;}
            writeLED(index);
            lcd.cls();
            lcd.printf("I received %s, at %d", receiveCodesEq[index], index);
            wait(0.5);
        }
        inputArray[0] = NULL;
        goto start;
    }
    int level = atoi(inputArray);
    lcd.printf(" %s", levelNames[level]);
    wait(3);
    lcd.cls();
    lcd.printf("Press OK when   You are ready!");
    IRget(14);


    while(true) {
        int index;
        int appearTime = ((10 - level) * (10 - level) * 25);
        int score = 0;
        round++;
        lcd.cls();
        lcd.printf("3...");
        wait(0.5);
        lcd.cls();
        lcd.printf("2..");
        wait(0.5);
        lcd.cls();
        lcd.printf("1.");
        wait(0.5);
        lcd.cls();
        lcd.printf("Catch'em!");
        lcd.locate(0, 1);
        wait(1);
        lcd.printf("Round: 0/5 ");
        for (int i = 0; i < 5; i++) {
            int randomNum = rand() % 10;
            float randomTime = (rand() % 40) / 10 + 1;
            wait(randomTime);
            writeLED(randomNum);
            index = IRReceive(appearTime);
            writeLED(17);
            wait(0.5);
            if ( index == randomNum )
            {
                lcd.printf("*");
                score++;
                writeLED(12);
            } else if ( index == -1 )
            {
                writeLED(16);
            } else
            {
                writeLED(14);
            }
            lcd.locate(7, 1);
            lcd.printf("%d", i + 1);
            lcd.locate(11 + score, 1);
            wait(1);
            writeLED(17);
            wait(1);
        }
        scoreTotal = scoreTotal + score;
        lcd.cls();
        lcd.printf("You scored %d/5  on level %d!", score, level);
        lcd.locate(14, 1);
        lcd.printf("->");
        IRget(15);
        lcd.cls();
        lcd.printf("Your success    rate is %.1f%%", float (scoreTotal/round) * 20);
        lcd.locate(14, 1);
        lcd.printf("->");
        IRget(15);
        lcd.cls();
        lcd.printf("Do you want");
        lcd.locate(0,1);
        lcd.printf("another round?");
        IRget(14);
        if (score > 3)
        {
            level++;
        } else {
            level--;
        }
        lcd.cls();
        lcd.printf("Level is set to");
        lcd.locate(0,1);
        lcd.printf("*%d %s", level, levelNames[level]);
        wait(2);
    }        
}

