/*!*********************************************************************************************************************
@file user_app1.c                                                                
@brief User's tasks / applications are written here.  This description
should be replaced by something specific to the task.

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- MUTE

CONSTANTS
- NONE

TYPES
- MorseCodeEntry (Struc)

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Structure Definitions
***********************************************************************************************************************/
typedef struct {
    u8 u8P_character[2];
    u8 u8P_morseCode[6];
} MorseCodeEntry;


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */
MorseCodeEntry G_MCE_MorseCodeList[] = {
        {"A", ".-"},    {"B", "-..."},  {"C", "-.-."},  {"D", "-.."},   {"E", "."},
        {"F", "..-."},  {"G", "--."},   {"H", "...."},  {"I", ".."},    {"J", ".---"},
        {"K", "-.-"},   {"L", ".-.."},  {"M", "--"},    {"N", "-."},    {"O", "---"},
        {"P", ".--."},  {"Q", "--.-"},  {"R", ".-."},   {"S", "..."},   {"T", "-"},
        {"U", "..-"},   {"V", "...-"},  {"W", ".--"},   {"X", "-..-"},  {"Y", "-.--"},
        {"Z", "--.."},  {"0", "-----"}, {"1", ".----"}, {"2", "..---"}, {"3", "...--"}, 
        {"4", "....-"}, {"5", "....."}, {"6", "-...."}, {"7", "--..."}, {"8", "---.."}, 
        {"9", "----."} 
}; 
volatile u32 G_u32_MorseCodeListDigitIndex = 26;
volatile bool G_b_MUTESOUND = FALSE;
volatile u32 G_u32_TimeCounter = 0;

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_pfStateMachine;               /*!< @brief The state machine function pointer */
//static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */

/**********************************************************************************************************************
Function Prototypes
**********************************************************************************************************************/
bool PlayMorseFromString(u8* MorseString, int duration);
int CharacterToMorseCodeListIndex(u8 character);
u8* CharacterToMorseCodeString(u8 character);
bool StartupSequence(void);
bool Game(void);

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
bool PlayMorseFromString(u8* MorseString, int duration){
  static u32 u32_TimeCounter = 0;
  static u8 u8_LoopCounter = 0;
  static u32 u32_Index = 0;
  static char char_CurrentCharacter;
  
  if(u32_TimeCounter >= duration)
  {
    u32_TimeCounter = 0;
    char_CurrentCharacter = MorseString[u32_Index];
    
    if(char_CurrentCharacter == '.')
    {
      if(u8_LoopCounter < 1)
      {
        LedOn(RED);
        if(!G_b_MUTESOUND)
        {
        PWMAudioOn(BUZZER1);
        }
        u8_LoopCounter++; // Increment LoopCounter
      }
      else
      {
        LedOff(RED);
        if(!G_b_MUTESOUND)
        {
        PWMAudioOff(BUZZER1);
        }
        u8_LoopCounter = 0; // Reset LoopCounter
        u32_Index++; // Increment Index
      }
        
    }
    else if(char_CurrentCharacter == '-')
    {
      if(u8_LoopCounter < 3)
      {
        LedOn(RED);
        if(!G_b_MUTESOUND)
        {
        PWMAudioOn(BUZZER1);
        }
        u8_LoopCounter++; // Increment LoopCounter
      }
      else
      {
        LedOff(RED);
        if(!G_b_MUTESOUND)
        {
        PWMAudioOff(BUZZER1);
        }
        u8_LoopCounter = 0; // Reset LoopCounter
        u32_Index++; // Increment Index
      }
    }
    else
    {
      LedOff(RED);
      if(!G_b_MUTESOUND)
      {
        PWMAudioOff(BUZZER1);
      }
      u32_TimeCounter = 0;
      u8_LoopCounter = 0;
      u32_Index = 0;
      return FALSE;
    }
  }
  
  u32_TimeCounter++; // Increment time
  
  return TRUE; // Continue Running
}

int CharacterToMorseCodeListIndex(u8 character){
  u32 u32_ASCIICode = character;
  if(u32_ASCIICode >= 65 && u32_ASCIICode <= 90) // "A" to "Z"
  {
    return u32_ASCIICode - 65; // Indexes 0-25
  }
  else if(u32_ASCIICode >= 97 && u32_ASCIICode <= 122) // "a" to "z"
  {
    return u32_ASCIICode - 97; // Indexes 0-25
  }
  else if(u32_ASCIICode >= 48 && u32_ASCIICode <= 57) // "0" to "9"
  {
    return u32_ASCIICode - 22; // Indexes 26-35
  }
  else // Out of bounds
  {
    return -1; 
  }
}

u8* CharacterToMorseCodeString(u8 character){
  u32 u32_MorseCodeListIndex = CharacterToMorseCodeListIndex(character);
  
  if(u32_MorseCodeListIndex == -1) {
    return "Out Of Bounds";
  }
  else
  {
    return G_MCE_MorseCodeList[u32_MorseCodeListIndex].u8P_morseCode;
  }
  
}

bool StartupSequence(){
  static u8 u8P_WelcomeMessageLine1[] = "Welcome to"; // buffer of 5 spaces from left to center
  static u8 u8P_WelcomeMessageLine2[] = "Morse Decoder"; // buffer of 3 from left to center
  static u8 u8P_LetterToWrite[] = {0,0};
  static u8 u8_IndexCounter = 0, u8_LineCounter = 1;
  static u32 u32_TimeCounter = 0;
  static bool b_PlayMorse = TRUE, b_ClearScreen = TRUE, b_CharWritten = FALSE;
  static bool b_ClearScreenCommandCalled = FALSE;
  static bool b_WelcomeComplete = FALSE, b_HowToPlayComplete  = TRUE, b_Stall = FALSE;
  
  u32_TimeCounter++;
  
  if(b_ClearScreen){
    LcdCommand(LCD_CLEAR_CMD);
    b_ClearScreen = FALSE;
  }
  
  if(b_Stall){
    if(u32_TimeCounter > 1500 && !b_ClearScreenCommandCalled){
      b_ClearScreen = TRUE;
      b_ClearScreenCommandCalled = TRUE;
    }
    if(u32_TimeCounter > 2000){
      b_Stall = FALSE;
      u32_TimeCounter = 0;
      b_ClearScreenCommandCalled = FALSE;
    }
    return TRUE;
  }
  
  // Welcome sequence
  if(!b_WelcomeComplete) {
    if(u8_LineCounter == 1){ // Line 1 Print
      if(u8_IndexCounter < 10){
        if(b_PlayMorse){
          if(!b_CharWritten){
            b_CharWritten = TRUE;
            u8P_LetterToWrite[0] = u8P_WelcomeMessageLine1[u8_IndexCounter];
            LcdMessage(LINE1_START_ADDR + 5 + u8_IndexCounter, u8P_LetterToWrite);
          }
          b_PlayMorse = PlayMorseFromString(CharacterToMorseCodeString(u8P_WelcomeMessageLine1[u8_IndexCounter]),40);
        }
        else if(u32_TimeCounter > 200){
          u8_IndexCounter++;
          b_PlayMorse = TRUE;
          b_CharWritten = FALSE;
          u32_TimeCounter = 0;
        }
      }
      else{
        u8_LineCounter++;
        u8_IndexCounter = 0;
      }
    }
    else if(u8_LineCounter == 2){ // Line 2 Print
       if(u8_IndexCounter < 13){
         if(b_PlayMorse){
           if(!b_CharWritten){
             b_CharWritten = TRUE;
             u8P_LetterToWrite[0] = u8P_WelcomeMessageLine2[u8_IndexCounter];
             LcdMessage(LINE2_START_ADDR + 3 + u8_IndexCounter, u8P_LetterToWrite);
           }
           b_PlayMorse = PlayMorseFromString(CharacterToMorseCodeString(u8P_WelcomeMessageLine2[u8_IndexCounter]),40);
         }
         else if(u32_TimeCounter > 100){
           u8_IndexCounter++;
           b_PlayMorse = TRUE;
           b_CharWritten = FALSE;
           u32_TimeCounter = 0;
         }
       }
       else{
         u8_IndexCounter = 0;
         u8_LineCounter = 0;
         b_WelcomeComplete = TRUE;
         b_HowToPlayComplete = FALSE;
         b_Stall = TRUE;
       }
    }
  }
  
  // How to play sequence
  /* --------------------
         How to play...    
                
     --------------------*/ 
  /* --------------------
     | Input              
     v  Morse             
     --------------------*/
  /* --------------------
         | Delete              
         v  Input              
     --------------------*/
  /* --------------------
        Restart |               
          Input v               
     --------------------*/
  /* --------------------
                 Enter  |               
               Solution v               
     --------------------*/
  if(!b_HowToPlayComplete){
    if(u8_IndexCounter == 0){
      LcdMessage(LINE1_START_ADDR,"    How to play...  ");
      LcdMessage(LINE2_START_ADDR,"                    ");
    }
    else if(u8_IndexCounter == 1){
      LcdMessage(LINE1_START_ADDR,"| Input             ");
      LcdMessage(LINE2_START_ADDR,"v  Morse            ");
    }
    else if(u8_IndexCounter == 2){
      LcdMessage(LINE1_START_ADDR,"     | Delete       ");
      LcdMessage(LINE2_START_ADDR,"     v  Input       ");
    }
    else if(u8_IndexCounter == 3){
      LcdMessage(LINE1_START_ADDR,"    Restart |       ");
      LcdMessage(LINE2_START_ADDR,"      Input v       ");
    }
    else if(u8_IndexCounter == 4){
      LcdMessage(LINE1_START_ADDR,"            Enter  |");
      LcdMessage(LINE2_START_ADDR,"          Solution v");
    }
    else{
      b_HowToPlayComplete = TRUE;
      u8_IndexCounter = 0;
    }
    u8_IndexCounter++;
    b_Stall = TRUE;
  }
  
  // End of function
  if(b_WelcomeComplete && b_HowToPlayComplete){
    // Finished running
    return FALSE;
  }
  else{
    // Continue running
    return TRUE;
  }
}

bool Game(){
  static u8 u8P_RoundString[] = "Round: ";
  static u32 u32_RandomNumber = 0, u32_RandomCharacterIndex;
  static u8 u8P_ExpectedMorseString[6] = {0,0,0,0,0,0}, u8P_CharacterToTranslate[2] = {0,0};
  static u16 u16_Index = 0;
  static bool b_RoundStart = TRUE;
  
  if(b_RoundStart){
    srand(G_u32_TimeCounter);
    u32_RandomNumber = rand();
    u32_RandomCharacterIndex = u32_RandomNumber % 26;
    u8P_CharacterToTranslate[0] = G_MCE_MorseCodeList[u32_RandomCharacterIndex].u8P_character[0];
    while(G_MCE_MorseCodeList[u32_RandomCharacterIndex].u8P_morseCode[u16_Index]){
      u8P_ExpectedMorseString[u16_Index] = G_MCE_MorseCodeList[u32_RandomCharacterIndex].u8P_morseCode[u16_Index];
      u16_Index++;
    }
    u16_Index = 0;
    LcdMessage(LINE1_START_ADDR,u8P_ExpectedMorseString);
    LcdMessage(LINE2_START_ADDR,u8P_CharacterToTranslate);
    b_RoundStart = FALSE;
  }
  
  return FALSE;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  /* Turn off all LEDs */
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  
  /* Setup buzzer */
  PWMAudioSetFrequency(BUZZER1, 500);
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void UserApp1RunActiveState(void)
{
  UserApp1_pfStateMachine();

} /* end UserApp1RunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* What does this state do? */
static void UserApp1SM_Idle(void)
{
  static bool b_Startup = TRUE;
  static bool b_Game = TRUE;
  static bool b_KeepRunning1 = FALSE;
  static bool b_KeepRunning2 = FALSE;
  static char* charP_MorseString1 = "...";
  static char* charP_MorseString2 = "----"; 
  
  G_u32_TimeCounter++;
  if(G_u32_TimeCounter > 10000){
    G_u32_TimeCounter = 0;
  }
  
  G_b_MUTESOUND = TRUE;
  
  if(b_Startup)
  {
    b_Startup = StartupSequence();
  }
  else if(b_Game){
    b_Game = Game();
  }
  else if(b_KeepRunning1)
  {
    b_KeepRunning1 = PlayMorseFromString(charP_MorseString1,100);
  }
  else if(b_KeepRunning2)
  {
    b_KeepRunning2 = PlayMorseFromString(charP_MorseString2,100);
  }
  else
  {
    LedOn(GREEN);
  }
    /*
  if(IsButtonPressed(BUTTON0))
  {
    PWMAudioOn(BUZZER1);
  }
  else
  {
    PWMAudioOff(BUZZER1);
  }
  
  char* charP_MorseCodeString = CharacterToMorseCodeString("A");
  */
} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
