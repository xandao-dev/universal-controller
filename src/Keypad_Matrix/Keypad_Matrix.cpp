#include <Arduino.h>        // for Arduino stuff like data types, pinMode, etc.
#include <limits.h>         // for CHAR_BIT
#include "Keypad_Matrix.h"  // for class definition, etc.

// constructor
Keypad_Matrix::Keypad_Matrix (const char *keyMap, const uint8_t *rowPins, const uint8_t *colPins,
                              const uint8_t numRows, const uint8_t numCols, const bool enablePullups)
  : keyMap_ (keyMap), rowPins_ (rowPins), colPins_ (colPins), numRows_ (numRows), numCols_ (numCols), enablePullups_ (enablePullups)

      {
      totalKeys_        = numRows * numCols;
      lastKeySetting_  = NULL;
      lastKeyTime_     = NULL;
      debounce_Time_    = 10;  // milliseconds
      // no handlers yet
      keyDownHandler_   = NULL;
      keyUpHandler_     = NULL;
      startRowHandler_  = startRow;
      endRowHandler_    = endRow;
      // default to doing a digitalRead
      readHandler_    = digitalRead;
      } // end of Keypad_Matrix::Keypad_Matrix

Keypad_Matrix::~Keypad_Matrix ()
  {
  if (lastKeySetting_)
    free (lastKeySetting_);
  if (lastKeyTime_)
    free (lastKeyTime_);

  // set each column to back to input
  if (enablePullups_)
    for (uint8_t i = 0; i < numCols_; i++)
      pinMode (colPins_ [i], INPUT);

  } // end Keypad_Matrix::~Keypad_Matrix

void Keypad_Matrix::begin ()
  {
  // if begin() already called, don't allocate memory again
  if (lastKeySetting_ != NULL)
    return;

  // allocate one bit per key, rounded up to next byte
  lastKeySetting_ = (char *) calloc ((totalKeys_ + CHAR_BIT - 1) / CHAR_BIT, 1);
  // allocate an unsigned long for the time the key last changed, for each key
  if (lastKeySetting_ != NULL)
    lastKeyTime_  = (unsigned long *) calloc (totalKeys_, sizeof (unsigned long));

  // give up if we couldn't allocate memory for both arrays
  if (lastKeyTime_ == NULL)
    {
    if (lastKeySetting_)
      free (lastKeySetting_);
    return;
    }

  // set each column to input-pullup (optional)
  if (enablePullups_)
    for (uint8_t i = 0; i < numCols_; i++)
      pinMode (colPins_ [i], INPUT_PULLUP);

  } // end of Keypad_Matrix::begin

void Keypad_Matrix::scan ()
  {
  // if Keypad_Matrix::begin has not been called then memory hasn't been allocated for the arrays
  if (lastKeySetting_   == NULL ||
      startRowHandler_  == NULL ||  // we need these handlers
      endRowHandler_    == NULL ||
      readHandler_      == NULL)
    return;

  uint8_t keyNumber = 0;          // current key number
  unsigned long now = millis ();  // for debouncing
  char keyChanged [(totalKeys_ + CHAR_BIT - 1) / CHAR_BIT];  // remember which ones changed
  memset (keyChanged, 0, sizeof (keyChanged));    // nothing yet
  bool changed = false;                           // did anything change? Not yet.

  // check each row
  for (uint8_t row = 0; row < numRows_; row++)
    {
    // handle start of a row
    // default: set that row to OUTPUT and LOW
    startRowHandler_ (rowPins_ [row]);

    // check each column to see if the switch has driven that column LOW
    for (uint8_t col = 0; col < numCols_; col++)
      {
      // debounce - ignore if not enough time has elapsed since last change
      if (now - lastKeyTime_ [keyNumber] >= debounce_Time_)
        {
        bool keyState = readHandler_ (colPins_ [col]) == LOW; // true means pressed
        if (keyState != (BITTEST (lastKeySetting_, keyNumber) != 0)) // changed?
          {
          lastKeyTime_ [keyNumber] = now;  // remember time it changed
          changed = true;                  // remember something changed
          BITSET (keyChanged, keyNumber);  // remember this key changed

          // remember new state
          if (keyState)
            BITSET   (lastKeySetting_, keyNumber);
          else
            BITCLEAR (lastKeySetting_, keyNumber);
          }  // if key state has changed
        }  // debounce time up
      keyNumber++;
      } // end of for each column

    // handle end of a row
    // default: put row back to high-impedance (input)
    endRowHandler_ (rowPins_ [row]);
    }  // end of for each row

  // If anything changed call the handlers.
  // We do this now in case the keys aren't polled very frequently. We have now
  // detected all the changes (first) before calling any handlers, in case the
  // handler wants to know of combinations like Ctrl+Z.
  if (changed)
    {
    // do key-ups first to make sure that combinations handled by external code are correct
    for (uint8_t i = 0; i < totalKeys_; i++)
      {
      if (BITTEST (keyChanged, i) != 0)  // did this one change?
        {
         if (BITTEST (lastKeySetting_, i) == 0)  // is now up
           if (keyUpHandler_)
             keyUpHandler_ (keyMap_ [i]);
        }  // end of this key changed
      }  // end of for each key

    // now do key-downs
    for (uint8_t i = 0; i < totalKeys_; i++)
      {
      if (BITTEST (keyChanged, i) != 0)  // did this one change?
        {
         if (BITTEST (lastKeySetting_, i) != 0)  // is now down
           if (keyDownHandler_)
             keyDownHandler_ (keyMap_ [i]);
        }  // end of this key changed
      }  // end of for each key
    }  // end of something changed

  } // end of Keypad_Matrix::scan

bool Keypad_Matrix::isKeyDown (const char which)
  {
  // if Keypad_Matrix::begin has not been called then memory hasn't been allocated for the arrays
  if (lastKeySetting_ == NULL)
    return false;

  // locate the desired key by a linear scan
  //  - this is a bit inefficient, but for a 16-key keypad it will be pretty fast
  for (uint8_t i = 0; i < totalKeys_; i++)
    {
    if (keyMap_ [i] == which)
      return BITTEST (lastKeySetting_, i) != 0;  // true if down
    }
  return false;   // that key isn't known
  } // end of Keypad_Matrix::isKeyDown

// default handler for starting a row
void Keypad_Matrix::startRow (uint8_t rowPin)
  {
  // set that row to OUTPUT and LOW
  pinMode (rowPin, OUTPUT);
  digitalWrite (rowPin, LOW);
  } // end of Keypad_Matrix::startRow

// default handler for ending a row
void Keypad_Matrix::endRow (uint8_t rowPin)
  {
  // put row back to high-impedance (input)
  pinMode (rowPin, INPUT);
  } // end of Keypad_Matrix::startRow
