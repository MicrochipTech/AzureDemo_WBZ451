/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ble_queue.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_ble_queue.h"
//#include "app.h"
#include "app_ble.h"
#include "led.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_BLE_QUEUE_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_BLE_QUEUE_DATA app_ble_queueData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_BLE_QUEUE_Initialize ( void )

  Remarks:
    See prototype in app_ble_queue.h.
 */

void APP_BLE_QUEUE_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_ble_queueData.state = APP_BLE_QUEUE_STATE_INIT;
    app_ble_queueData.appQueue = xQueueCreate(20, sizeof (APP_Msg_T));


    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

uint16_t conn_hdl; // connection handle info captured @BLE_GAP_EVT_CONNECTED event
extern char*       attDeviceID;


void parseCmd(uint8_t* pWrBuffer, size_t size)
{

    if (strstr(pWrBuffer, "+LED_RED:ON"))
    {
        RED_LED_Set();
        return;
    }
    if (strstr(pWrBuffer, "+LED_RED:OFF"))
    {
        RED_LED_Clear();
        return;
    }
    if (strstr(pWrBuffer, "+LED_GREEN:ON"))
    {
        GREEN_LED_Set();
        return;
    }
    if (strstr(pWrBuffer, "+LED_GREEN:OFF"))
    {
        GREEN_LED_Clear();
        return;
    }
    if (strstr(pWrBuffer, "+LED_BLUE:ON"))
    {
        BLUE_LED_Set();
        return;
    }
    if (strstr(pWrBuffer, "+LED_BLUE:OFF"))
    {
        BLUE_LED_Clear();
        return;
    }
    if (strstr(pWrBuffer, "+MSG:"))
    {
        pWrBuffer += strlen("+MSG:");
        size -= strlen("+MSG:");
        /*LUC_AZURE*/
        //process_telemetry_command(1, pWrBuffer);
        //BLUE_LED_Toggle();
        process_sendString_command(pWrBuffer);
        /*LUC_AZURE*/
        
        return;
    }
    if (strstr(pWrBuffer, "+STATUS:"))
    {
        uint8_t buffer[25];
        //static uint32_t count = 0;

        //Send status back to BLE device with RGB value  RGB: R g B  Upper case is ON lower case is OFF
        sprintf(buffer, "+RGB: %s %s %s", RED_LED_Get() == 1 ? "R" : "r", GREEN_LED_Get() == 1 ? "G" : "g", BLUE_LED_Get() == 1 ? "B" : "b");
        size = strlen(buffer);

        BLE_TRSPS_SendData(conn_hdl, size, buffer);
        return;
    }
    if (strstr(pWrBuffer, "+SN:"))
    {
        uint8_t buffer[35];
        //static uint32_t count = 0;

        
        //Send status back to BLE device with RGB value  RGB: R g B  Upper case is ON lower case is OFF
        sprintf(buffer, "%s", attDeviceID);
        size = strlen(buffer);

        BLE_TRSPS_SendData(conn_hdl, size, buffer);
        return;
    }
    //    if (strstr(pWrBuffer, "time"))
    //        getTime();
    //    if (strstr(pWrBuffer, "disconnect"))
    //        disconnect();
    //displayOnWebPage(pWrBuffer, size);
};
void cloud2ble(uint8_t *message)
{
    uint8_t buffer [100];
    sprintf (buffer,"%s",message);
    BLE_TRSPS_SendData(conn_hdl, strlen(buffer), buffer);
};


/******************************************************************************
  Function:
    void APP_BLE_QUEUE_Tasks ( void )

  Remarks:
    See prototype in app_ble_queue.h.
 */

void APP_BLE_QUEUE_Tasks ( void )
{
    APP_Msg_T appMsg[1];
    APP_Msg_T *p_appMsg;
    p_appMsg = appMsg;

    /* Check the application's current state. */
    switch ( app_ble_queueData.state )
    {
        /* Application's initial state. */
        case APP_BLE_QUEUE_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {
                APP_BleStackInit();
                //BLE_GAP_SetDeviceName(strlen ("WBZ451_WINC"),"WBZ451_WINC");
                // Start Advertisement
                BLE_GAP_SetAdvEnable(0x01, 0x00);
                SERCOM0_USART_Write((uint8_t *) "\r\nAdvertising\r\n", 13);

                app_ble_queueData.state = APP_BLE_QUEUE_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_BLE_QUEUE_STATE_SERVICE_TASKS:
        {
            //(GPIOB_REGS->GPIO_LATINV= (1<<5));//BLUE_LED_Toggle();
            if (OSAL_QUEUE_Receive(&app_ble_queueData.appQueue, &appMsg, OSAL_WAIT_FOREVER))
            {
                if (p_appMsg->msgId == APP_MSG_BLE_STACK_EVT)
                {
                    // Pass BLE Stack Event Message to User Application for handling
                    APP_BleStackEvtHandler((STACK_Event_T *) p_appMsg->msgData);
                } else if (p_appMsg->msgId == APP_MSG_BLE_STACK_LOG)
                {
                    // Pass BLE LOG Event Message to User Application for handling
                    APP_BleStackLogHandler((BT_SYS_LogEvent_T *) p_appMsg->msgData);
                } 
            }
            LED_BLUE_Toggle_EX();
            
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
