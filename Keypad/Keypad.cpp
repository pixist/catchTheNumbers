#include "Keypad.h"
#include "mbed.h"


// Keypad layout:
//                [row][col]   Col0 Col1 Col2 Col3 
char const kpdLayout[4][4] = {{'1' ,'2' ,'3' ,'A'},  //row0
                              {'4' ,'5' ,'6' ,'B'},  //row1
                              {'7' ,'8' ,'9' ,'C'},  //row2
                              {'*' ,'0' ,'#' ,'D'}}; //row3

//NIBBLE LOW=0000,  HIGH= 0111 1011 1101 1110    Col  (x)
//const char KpdInMask[4] ={0xe0,0xd0,0xb0,0x70};
const char KpdInMask[4] ={0x0e,0x0d,0x0b,0x07};

//NIBBLE HIGH=1111,  LOW= 0111 1011 1101 1110    Rows (y)
//const char KpdOutMask[4]={0xfe,0xfd,0xfb,0xf7};  
const char KpdOutMask[4]={0x0e,0x0d,0x0b,0x07};


Keypad::Keypad(PinName col1, PinName col2, PinName col3, PinName col4, PinName row1, PinName row2, PinName row3, PinName row4) : _cols(col1,col2,col3,col4), _rows(row1,row2,row3,row4) {   }

char Keypad::ReadKey() {
    char KeyValue, Done=0;
    uint16_t y, x;
    
    //delay_ms(ContactBounceTime);  //warning no contact bounce protection
                                    //call read_key more than once with delay
                                    //between if key stay constant then key is pressed
    y = 0;
    while((y < 4) && (!Done))
    {
        _cols = KpdOutMask[y];              //write mask value to the column outputs     
        wait(0.01);  

        KeyValue = _rows;                   //read mask value from the row inputs
        
        if(KeyValue == KpdInMask[0])
            x = 0;
        else if(KeyValue == KpdInMask[1])
            x = 1;
        else if(KeyValue == KpdInMask[2])
            x = 2;
        else if(KeyValue == KpdInMask[3])
            x = 3;
        else
        {
            KeyValue='\0';                  //more than one key was pressed or no key in this row.
            x=9;          
        }
        if(x != 9)
        {
            Done = 1;                       //valid key found
            KeyValue = kpdLayout[x][y];     //convert to a character eg. '1','2','3','#','*'
        }
        y++;
    }
    return(KeyValue);
}