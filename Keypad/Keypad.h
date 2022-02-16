/* Keypad Library v1.0
 * Copyright (c) 2016 Grant Phillips
 * grant.phillips@nmmu.ac.za
 *
 * This is a modified version of Riaan Ehlers' library which was written for the 
 * Microchip PIC18 series microcontrollers.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#ifndef Keypad_H
#define Keypad_H
 
#include "mbed.h"
 
/** Class library for a polling-based 4x4 keypad.
 * 
 * Example:
 * @code
 * #include "mbed.h"
 * #include "Keypad.h"
 *
 * Keypad kpad(PE_15, PE_14, PE_13, PE_12, PE_11, PE_10, PE_9, PE_8);
 *
 * int main() {
 *     char key;
 *     int released = 1;
 *
 *     while(1){
 *         key = kpad.ReadKey();                   //read the current key pressed
 *
 *         if(key == '\0')
 *             released = 1;                       //set the flag when all keys are released
 *            
 *         if((key != '\0') && (released == 1)) {  //if a key is pressed AND previous key was released
 *             printf("%c\n", key);            
 *             released = 0;                       //clear the flag to indicate that key is still pressed
 *         }
 *     }
 * }
 * @endcode
 */
 
class Keypad {
  public:
    /** Create a Keypad object.
    * @param col1..4 DigitalOut (or BusOut) compatible pins for keypad Column data lines
    * @param row1..4 DigitalIn (or BusIn) compatible pins for keypad Row data lines
    */
    Keypad(PinName col1, PinName col2, PinName col3, PinName col4, PinName row1, PinName row2, PinName row3, PinName row4);
    
    /** Returns the letter of the key pressed.
    * @param 
    *     None
    * @return 
    *     The character of the key pressed.  Returns '\0' if no key is pressed.
    */
    char ReadKey();
    
 
  private:
    BusOut _cols;    
    BusIn _rows;
};
 
#endif