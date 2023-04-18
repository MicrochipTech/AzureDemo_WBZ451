/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble_queue.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_BLE_QUEUE_Initialize" and "APP_BLE_QUEUE_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_BLE_QUEUE_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_BLE_QUEUE_H
#define _APP_BLE_QUEUE_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "osal/osal_freertos_extend.h"


#include "azure/core/az_span.h"
#include "azure/core/az_json.h"
#include "azure/iot/az_iot_pnp_client.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_BLE_QUEUE_STATE_INIT=0,
    APP_BLE_QUEUE_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_BLE_QUEUE_STATES;

typedef enum APP_MsgId_T
{
    APP_MSG_BLE_STACK_EVT,
    APP_MSG_BLE_STACK_LOG,
    APP_MSG_ZB_STACK_EVT,
    APP_MSG_ZB_STACK_CB,
    APP_MSG_UART_CB,        
    APP_MSG_STACK_END
} APP_MsgId_T;

typedef struct APP_Msg_T
{
    APP_MsgId_T msgId;
    uint8_t msgData[32];   //No need to be 256,  APP_MSG_BLE_STACK_EVT only pushes ID (8), lenght(16) and pointer (32), 7 bytes....
                            //  refer in app_ble.c  to APP_BleStackCb 
} APP_Msg_T;
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_BLE_QUEUE_STATES state;

    /* TODO: Define any additional data used by the application. */
    OSAL_QUEUE_HANDLE_TYPE appQueue;

} APP_BLE_QUEUE_DATA;

extern APP_BLE_QUEUE_DATA app_ble_queueData;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BLE_QUEUE_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_BLE_QUEUE_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_BLE_QUEUE_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_BLE_QUEUE_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_BLE_QUEUE_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_BLE_QUEUE_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_BLE_QUEUE_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_BLE_QUEUE_H */

/*******************************************************************************
 End of File
 */

