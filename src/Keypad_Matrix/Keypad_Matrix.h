typedef void (*keyHandler) (const char which);
typedef int  (*colHandler) (uint8_t);
typedef void (*rowHandler) (uint8_t);

#define makeKeymap(x) ((const char*)x)  // for passing a multi-dimensional array to the constructor

class Keypad_Matrix {
  const char * keyMap_;     // array mapping key numbers to a character (eg. 0,0 is 'A')
  const uint8_t * rowPins_; // array of row pin numbers
  const uint8_t * colPins_; // array of column pin numbers
  uint8_t numRows_;
  uint8_t numCols_;
  bool enablePullups_;
  uint8_t totalKeys_;      // numRows_ * numCols_
  char * lastKeySetting_;  // bitmap of last setting of each key
  unsigned long * lastKeyTime_;   // when each key was last changed
  unsigned long debounce_Time_;   // how long to debounce for
  // event handlers (callback functions)
  keyHandler keyDownHandler_;  // handler for keydown event
  keyHandler keyUpHandler_;    // handler for keyup event
  rowHandler startRowHandler_; // handler called at the start of each row
  rowHandler endRowHandler_;   // handler called at the end of each row
  colHandler readHandler_;     // handler for reading a column

  // default handlers for starting and ending a row
  static void startRow (uint8_t rowPin);
  static void endRow   (uint8_t rowPin);

public:

  // constructor
  Keypad_Matrix (const char * keyMap, const uint8_t *rowPins, const uint8_t *colPins,
                 const uint8_t numRows, const uint8_t numCols, const bool enablePullups = true);
  // destructor
  ~Keypad_Matrix ();

  // public functions
  void begin ();        // call to initialize (set pullups, allocate memory)
  void scan ();         // call periodically to scan the keys
  bool isKeyDown            (const char which);  // see if a certain key is down
  void setKeyDownHandler    (keyHandler handler)  { keyDownHandler_   = handler; }  // handle key down
  void setKeyUpHandler      (keyHandler handler)  { keyUpHandler_     = handler; }  // handle key up
  void setRowHandlers       (rowHandler start, rowHandler end)
                              {
                              startRowHandler_  = start;
                              endRowHandler_    = end;
                              }  // row handlers
  void setColumnReadHandler (colHandler handler)  { readHandler_      = handler; }  // read a column
  void setDebounceTime      (const unsigned long debounce_Time) { debounce_Time_ = debounce_Time; }
};  // end of Keypad_Matrix class


// See: http://c-faq.com/misc/bitsets.html

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
