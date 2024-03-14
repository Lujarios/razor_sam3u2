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
- NONE

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
    char* charP_character;
    char* charP_morseCode;
} MorseCodeEntry;


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */
volatile MorseCodeEntry G_MCE_MorseCodeList[] = {
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
bool PlayMorseFromString(char* MorseString);
int CharacterToMorseCodeListIndex(char* character);
char* CharacterToMorseCodeString(char* character);

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
bool PlayMorseFromString(char* MorseString){
  static u32 u32_TimeCounter = 0;
  static u8 u8_LoopCounter = 0;
  static u32 u32_Index = 0;
  static char char_CurrentCharacter = ' ';
  
  if(u32_TimeCounter >= 400)
  {
    u32_TimeCounter = 0;
    char_CurrentCharacter = MorseString[u32_Index];
    
    if(char_CurrentCharacter == '.')
    {
      if(u8_LoopCounter < 1)
      {
        LedOn(RED);
        PWMAudioOn(BUZZER1);
        u8_LoopCounter++; // Increment LoopCounter
      }
      else
      {
        LedOff(RED);
        PWMAudioOff(BUZZER1);
        u8_LoopCounter = 0; // Reset LoopCounter
        u32_Index++; // Increment Index
      }
        
    }
    else if(char_CurrentCharacter == '-')
    {
      if(u8_LoopCounter < 3)
      {
        LedOn(RED);
        PWMAudioOn(BUZZER1);
        u8_LoopCounter++; // Increment LoopCounter
      }
      else
      {
        LedOff(RED);
        PWMAudioOff(BUZZER1);
        u8_LoopCounter = 0; // Reset LoopCounter
        u32_Index++; // Increment Index
      }
    }
    else
    {
      return FALSE;
    }
  }
  
  u32_TimeCounter++; // Increment time
  
  return TRUE; // Continue Running
}

int CharacterToMorseCodeListIndex(char* character){
  u32 u32_ASCIICode = *character;
  if(u32_ASCIICode >= 65 && u32_ASCIICode <= 90) // "A" to "Z"
  {
    return u32_ASCIICode - 65; // Indexes 0-25
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

char* CharacterToMorseCodeString(char* character){
  u32 u32_MorseCodeListIndex = CharacterToMorseCodeListIndex(character);
  if(u32_MorseCodeListIndex == -1) {
    return "Out Of Bounds";
  }
  else
  {
    return G_MCE_MorseCodeList[u32_MorseCodeListIndex].charP_morseCode;
  }
  
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
  
  bool b_KeepRunning = TRUE;
  char* charP_MorseString = ".--";
  
  while(b_KeepRunning)
  {
    b_KeepRunning = PlayMorseFromString(charP_MorseString);
  }
    
} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
