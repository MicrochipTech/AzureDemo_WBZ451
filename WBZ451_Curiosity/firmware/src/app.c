/*******************************************************************************
 * Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app_ble.h"
#include "ble_trsps/ble_trsps.h"

#include "app.h"
#include "wdrv_winc_client_api.h"
#include "iot_config/IoT_Sensor_Node_config.h"
#include "services/iot/cloud/crypto_client/cryptoauthlib_main.h"
#include "services/iot/cloud/crypto_client/crypto_client.h"
#include "services/iot/cloud/cloud_service.h"
#include "services/iot/cloud/wifi_service.h"
#include "services/iot/cloud/bsd_adapter/bsdWINC.h"
#include "credentials_storage/credentials_storage.h"
#include "debug_print.h"
#include "led.h"
#include "azutil.h"
#include "services/iot/cloud/mqtt_packetPopulation/mqtt_packetPopulate.h"
#include "services/iot/cloud/mqtt_packetPopulation/mqtt_iothub_packetPopulate.h"
#include "services/iot/cloud/mqtt_packetPopulation/mqtt_iotprovisioning_packetPopulate.h"

#if CFG_ENABLE_CLI
#include "system/command/sys_command.h"
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Local Function Prototypes
// *****************************************************************************
// *****************************************************************************
static void APP_SendToCloud(void);
static void APP_DataTask(void);
static void APP_WiFiConnectionStateChanged(uint8_t status);
static void APP_ProvisionRespCb(DRV_HANDLE handle, WDRV_WINC_SSID* targetSSID, WDRV_WINC_AUTH_CONTEXT* authCtx, bool status);
static void APP_DHCPAddressEventCb(DRV_HANDLE handle, uint32_t ipAddress);
static void APP_GetTimeNotifyCb(DRV_HANDLE handle, uint32_t timeUTC);
static void APP_ConnectNotifyCb(DRV_HANDLE handle, WDRV_WINC_ASSOC_HANDLE assocHandle, WDRV_WINC_CONN_STATE currentState, WDRV_WINC_CONN_ERROR errorCode);
static void APP_WifiScanTask(DRV_HANDLE handle);


void HeartBeat_CallBack_On(uintptr_t context);
void HeartBeat_CallBack_Off(uintptr_t context);

static char APP_WiFiApList[100];//[SERCOM0_USART_WRITE_BUFFER_SIZE - 1];

static char* LED_Property[3] = {
    "On",
    "Off",
    "Blink",
};

static uint8_t RGB_LED_DutyCycle[3];

#ifdef CFG_MQTT_PROVISIONING_HOST
void iot_provisioning_completed(void);
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Application Macros
// *****************************************************************************
// *****************************************************************************

#define APP_WIFI_SOFT_AP         0
#define APP_WIFI_DEFAULT         1
#define APP_DATATASK_INTERVAL    250L // Each unit is in msec
#define APP_CLOUDTASK_INTERVAL   APP_DATATASK_INTERVAL
#define APP_SW_DEBOUNCE_INTERVAL 1460000L

/* WIFI SSID, AUTH and PWD for AP */
#define APP_CFG_MAIN_WLAN_SSID ""
#define APP_CFG_MAIN_WLAN_AUTH M2M_WIFI_SEC_WPA_PSK
#define APP_CFG_MAIN_WLAN_PSK  ""

//#define CFG_APP_WINC_DEBUG 1   //define this to print WINC debug messages

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define SN_STRING "sn"

/* Place holder for ECC608A unique serial id */
ATCA_STATUS appCryptoClientSerialNumber;
char*       attDeviceID;
char        attDeviceID_buf[25] = "BAAAAADD1DBAAADD1D";
char        deviceIpAddress[16] = "0.0.0.0";

shared_networking_params_t shared_networking_params;

/* Various NTP Host servers that application relies upon for time sync */
#define WORLDWIDE_NTP_POOL_HOSTNAME "*.pool.ntp.org"
#define ASIA_NTP_POOL_HOSTNAME      "asia.pool.ntp.org"
#define EUROPE_NTP_POOL_HOSTNAME    "europe.pool.ntp.org"
#define NAMERICA_NTP_POOL_HOSTNAME  "north-america.pool.ntp.org"
#define OCEANIA_NTP_POOL_HOSTNAME   "oceania.pool.ntp.org"
#define SAMERICA_NTP_POOL_HOSTNAME  "south-america.pool.ntp.org"
#define NTP_HOSTNAME                "pool.ntp.org"

/* Driver handle for WINC1510 */
static DRV_HANDLE wdrvHandle;
static uint8_t    wifi_mode = WIFI_DEFAULT;

static SYS_TIME_HANDLE App_DataTaskHandle      = SYS_TIME_HANDLE_INVALID;
volatile bool          App_DataTaskTmrExpired  = false;
static SYS_TIME_HANDLE App_CloudTaskHandle     = SYS_TIME_HANDLE_INVALID;
volatile bool          App_CloudTaskTmrExpired = false;
volatile bool          App_WifiScanPending     = false;

static time_t     previousTransmissionTime;
volatile uint32_t telemetryInterval = CFG_DEFAULT_TELEMETRY_INTERVAL_SEC;

volatile bool iothubConnected = false;

extern pf_MQTT_CLIENT    pf_mqtt_iotprovisioning_client;
extern pf_MQTT_CLIENT    pf_mqtt_iothub_client;
extern void              sys_cmd_init();
extern userdata_status_t userdata_status;

#ifdef IOT_PLUG_AND_PLAY_MODEL_ID
extern az_iot_pnp_client pnp_client;
#else
extern az_iot_hub_client iothub_client;
#endif

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// * PnP Values *
// The model id is the JSON document (also called the Digital Twins Model Identifier or DTMI)
// which defines the capability of your device. The functionality of the device should match what
// is described in the corresponding DTMI. Should you choose to program your own PnP capable device,
// the functionality would need to match the DTMI and you would need to update the below 'model_id'.
// Please see the sample README for more information on this DTMI.
#ifdef IOT_PLUG_AND_PLAY_MODEL_ID
const az_span device_model_id_span = AZ_SPAN_LITERAL_FROM_STR(IOT_PLUG_AND_PLAY_MODEL_ID);
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void APP_CloudTaskcb(uintptr_t context)
{
    App_CloudTaskTmrExpired = true;
}

void APP_DataTaskcb(uintptr_t context)
{
    App_DataTaskTmrExpired = true;
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

// React to the WIFI state change here. Status of 1 means connected, Status of 0 means disconnected
static void APP_WiFiConnectionStateChanged(uint8_t status)
{
    // debug_printInfo("  APP: WiFi Connection Status Change to %d", status);
    //  If we have no AP access we want to retry
    if (status != 1)
    {
        // Restart the WIFI module if we get disconnected from the WiFi Access Point (AP)
        CLOUD_reset();
    }
}
// *****************************************************************************
// *****************************************************************************
// Section: Button interrupt handlers
// *****************************************************************************
// *****************************************************************************
void APP_SW0_Handler(void)
{
    //LED_ToggleRed();
    button_press_data.sw0_press_count++;
    button_press_data.flag.sw0 = 1;
}

void APP_SW1_Handler(void)
{
    //LED_ToggleRed();
    button_press_data.sw1_press_count++;
    button_press_data.flag.sw1 = 1;
}

void APP_SW2_Handler(void)
{
    //LED_ToggleRed();
    button_press_data.sw2_press_count++;
    button_press_data.flag.sw2 = 1;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */
extern uint16_t conn_hdl; // connection handle info captured @BLE_GAP_EVT_CONNECTED event
uint8_t uart_data;

void APP_Initialize(void) {
    debug_printInfo("  APP: %s()", __FUNCTION__);
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_CRYPTO_INIT;
    appData.scanState = STATE_SCAN_INIT;

    SYS_TIME_CallbackRegisterMS((SYS_TIME_CALLBACK) HeartBeat_CallBack_Off, 0, 50 /*BLINK_RATE_MS*/, SYS_TIME_SINGLE);
    
    // uint8_t wifi_mode = WIFI_DEFAULT;
    uint32_t sw0CurrentVal = 0;
    uint32_t sw1CurrentVal = 0;
    uint32_t i = 0;

    previousTransmissionTime = 0;
    //appData.appQueue = xQueueCreate(8, sizeof (APP_Msg_T));
    
    
    debug_init(attDeviceID);
    LED_init();
    //LED_test();
    // Blocking debounce
    for (i = 0; i < APP_SW_DEBOUNCE_INTERVAL; i++) {
        sw0CurrentVal += 1; //SW0_GPIO_PA00_Get();
        sw1CurrentVal += 1; //SW1_GPIO_PA01_Get();
    }

    if (sw0CurrentVal < (APP_SW_DEBOUNCE_INTERVAL / 2)) {
        if (sw1CurrentVal < (APP_SW_DEBOUNCE_INTERVAL / 2)) {
            strcpy(ssid, APP_CFG_MAIN_WLAN_SSID);
            strcpy(pass, APP_CFG_MAIN_WLAN_PSK);
            sprintf((char*) authType, "%d", APP_CFG_MAIN_WLAN_AUTH);
        } else {
            wifi_mode = WIFI_SOFT_AP;
        }
    }
    /* Open I2C driver client */
    ADCHS_ModulesEnable(0xffff);
    //LED_test();
    sys_cmd_init(); // CLI init

#if (CFG_APP_WINC_DEBUG == 1)
    WDRV_WINC_DebugRegisterCallback(debug_printer);
#endif
    //    EIC_CallbackRegister(EIC_PIN_0, (EIC_CALLBACK)APP_SW0_Handler, 0);
    //    EIC_InterruptEnable(EIC_PIN_0);
    //    EIC_CallbackRegister(EIC_PIN_1, (EIC_CALLBACK)APP_SW1_Handler, 0);
    //    EIC_InterruptEnable(EIC_PIN_1);

    userdata_status.as_uint8 = 0;
}

static void APP_ConnectNotifyCb(DRV_HANDLE handle, WDRV_WINC_ASSOC_HANDLE assocHandle, WDRV_WINC_CONN_STATE currentState, WDRV_WINC_CONN_ERROR errorCode) {
    debug_printTrace("  APP: APP_ConnectNotifyCb %d", currentState);
    WDRV_WINC_SSID ssid;

    if (WDRV_WINC_CONN_STATE_CONNECTED == currentState) {
        WiFi_ConStateCb(M2M_WIFI_CONNECTED);
        WDRV_WINC_AssociationSSIDGet(handle, &ssid, NULL);
    } else if (WDRV_WINC_CONN_STATE_DISCONNECTED == currentState) {
        switch (errorCode) {
            case WDRV_WINC_CONN_ERROR_SCAN:
                debug_printError("  WiFi: DISCONNECTED. Failed to detect AP during scan. Check SSID & authType");
                break;
            case WDRV_WINC_CONN_ERROR_AUTH:
                debug_printError("  WiFi: DISCONNECTED. Failed to authenticate. Check pass phrase");
                break;
            case WDRV_WINC_CONN_ERROR_INPROGRESS:
                break;
            case WDRV_WINC_CONN_ERROR_ASSOC:
            case WDRV_WINC_CONN_ERROR_NOCRED:
            case WDRV_WINC_CONN_ERROR_UNKNOWN:
            default:
                debug_printError("  WiFi: DISCONNECTED, Error Code: %d.  Check WiFi Credentials", errorCode);
        }
        WiFi_ConStateCb(M2M_WIFI_DISCONNECTED);
    }
}

static void APP_GetTimeNotifyCb(DRV_HANDLE handle, uint32_t timeUTC) {
    // checking > 0 is not recommended, even if getsystime returns null, utctime value will be > 0
    if (timeUTC != 0x86615400U) {
        tstrSystemTime pSysTime;
        struct tm theTime;

        WDRV_WINC_UTCToLocalTime(timeUTC, &pSysTime);
        theTime.tm_hour = pSysTime.u8Hour;
        theTime.tm_min = pSysTime.u8Minute;
        theTime.tm_sec = pSysTime.u8Second;
        theTime.tm_year = pSysTime.u16Year - 1900;
        theTime.tm_mon = pSysTime.u8Month - 1;
        theTime.tm_mday = pSysTime.u8Day;
        theTime.tm_isdst = 0;
        RTC_RTCCTimeSet(&theTime);
    }
}

static void APP_DHCPAddressEventCb(DRV_HANDLE handle, uint32_t ipAddress) {
    //LED_SetWiFi(LED_INDICATOR_SUCCESS);

    memset(deviceIpAddress, 0, sizeof (deviceIpAddress));

    sprintf(deviceIpAddress, "%lu.%lu.%lu.%lu",
            (0x0FF & (ipAddress)),
            (0x0FF & (ipAddress >> 8)),
            (0x0FF & (ipAddress >> 16)),
            (0x0FF & (ipAddress >> 24)));

    debug_printGood("  APP: DHCP IP Address %s", deviceIpAddress);

    shared_networking_params.haveIpAddress = 1;
    shared_networking_params.haveERROR = 0;
    shared_networking_params.reported = 0;
}

static void APP_ProvisionRespCb(DRV_HANDLE handle,
        WDRV_WINC_SSID* targetSSID,
        WDRV_WINC_AUTH_CONTEXT* authCtx,
        bool status) {
    uint8_t sectype;
    uint8_t* ssid;
    uint8_t* password;

    debug_printInfo("  APP: %s()", __FUNCTION__);

    if (status == M2M_SUCCESS) {
        sectype = authCtx->authType;
        ssid = targetSSID->name;
        password = authCtx->authInfo.WPAPerPSK.key;
        WiFi_ProvisionCb(sectype, ssid, password);
    }
}

void APP_Tasks(void) {
    APP_Msg_T appMsg[1];
    APP_Msg_T *p_appMsg;
    p_appMsg = appMsg;


    switch (appData.state) {
        case APP_STATE_CRYPTO_INIT:
        {

            char serialNumber_buf[25];

            shared_networking_params.allBits = 0;
            debug_setPrefix(attDeviceID);
            cryptoauthlib_init();
            transfer_ecc_certs_to_winc();

            if (cryptoDeviceInitialized == false) {
                debug_printError("  APP: CryptoAuthInit failed");
            }

#ifdef HUB_DEVICE_ID
            attDeviceID = HUB_DEVICE_ID;
#else
            appCryptoClientSerialNumber = CRYPTO_CLIENT_printSerialNumber(serialNumber_buf);
            if (appCryptoClientSerialNumber != ATCA_SUCCESS) {
                switch (appCryptoClientSerialNumber) {
                    case ATCA_GEN_FAIL:
                        debug_printError("  APP: DeviceID generation failed, unspecified error");
                        break;
                    case ATCA_BAD_PARAM:
                        debug_printError("  APP: DeviceID generation failed, bad argument");
                    default:
                        debug_printError("  APP: DeviceID generation failed");
                        break;
                }
            } else {
                // To use Azure provisioning service, attDeviceID should match with the device cert CN,
                // which is the serial number of ECC608 prefixed with "sn" if you are using the
                // the microchip provisioning tool for PIC24.
#if(1)
                strcpy(attDeviceID_buf, SN_STRING);
                strcat(attDeviceID_buf, serialNumber_buf);
                attDeviceID = attDeviceID_buf;
#else
                strcpy(attDeviceID_buf, serialNumber_buf);
                strcat(attDeviceID_buf," ATECC" );
                attDeviceID = attDeviceID_buf;
#endif
            }
#endif
#if CFG_ENABLE_CLI
            set_deviceId(attDeviceID);
#endif
            debug_setPrefix(attDeviceID);
            CLOUD_setdeviceId(attDeviceID);
            appData.state = APP_STATE_WDRV_INIT;
            break;
        }

        case APP_STATE_WDRV_INIT:
        {
            if (SYS_STATUS_READY == WDRV_WINC_Status(sysObj.drvWifiWinc)) {
                appData.state = APP_STATE_WDRV_INIT_READY;

            }

            break;
        }

        case APP_STATE_WDRV_INIT_READY:
        {
            wdrvHandle = WDRV_WINC_Open(0, 0);

            if (DRV_HANDLE_INVALID != wdrvHandle) {
                appData.state = APP_STATE_WDRV_OPEN;
            }

            break;
        }

        case APP_STATE_WDRV_OPEN:
        {
            m2m_wifi_configure_sntp((uint8_t*) NTP_HOSTNAME, strlen(NTP_HOSTNAME), SNTP_ENABLE_DHCP);
            m2m_wifi_enable_sntp(1);
            WDRV_WINC_DCPT* pDcpt = (WDRV_WINC_DCPT*) wdrvHandle;
            pDcpt->pCtrl->pfProvConnectInfoCB = APP_ProvisionRespCb;

            debug_printInfo("  APP: WiFi Mode %d", wifi_mode);
            wifi_init(APP_WiFiConnectionStateChanged, wifi_mode);

#ifdef CFG_MQTT_PROVISIONING_HOST
            pf_mqtt_iotprovisioning_client.MQTT_CLIENT_task_completed = iot_provisioning_completed;
            CLOUD_init_host(CFG_MQTT_PROVISIONING_HOST, attDeviceID, &pf_mqtt_iotprovisioning_client);
#else
            CLOUD_init_host(hub_hostname, attDeviceID, &pf_mqtt_iothub_client);
#endif   // CFG_MQTT_PROVISIONING_HOST
#if(_ELIMINATE)
#define SSID "NETGEAR51"
#define PWD "melodicship232"
#define MODE "2"
            if (wifi_mode == WIFI_DEFAULT) {
                WDRV_WINC_AUTH_CONTEXT authCtx;
                WDRV_WINC_BSS_CONTEXT bssCtx;
                /* Enable use of DHCP for network configuration, DHCP is the default
                but this also registers the callback for notifications. */
                WDRV_WINC_IPUseDHCPSet(wdrvHandle, &APP_DHCPAddressEventCb);

                printf("  APP: registering APP_CloudTaskcb");
                App_CloudTaskHandle = SYS_TIME_CallbackRegisterMS(APP_CloudTaskcb, 0, APP_CLOUDTASK_INTERVAL, SYS_TIME_PERIODIC);
                if (strlen(SSID > 0)) {
                    if (WDRV_WINC_STATUS_OK != WDRV_WINC_BSSCtxSetDefaults(&bssCtx)) {
                        break;
                    }
                    if (WDRV_WINC_STATUS_OK != WDRV_WINC_BSSCtxSetSSID(&bssCtx, (uint8_t*) SSID, strlen(SSID))) {
                        break;
                    };
                    if (WDRV_WINC_STATUS_OK != WDRV_WINC_AuthCtxSetWPA(&authCtx, (uint8_t*) PWD, strlen(PWD))) {
                        break;
                    }
                    WDRV_WINC_STATUS_OK == WDRV_WINC_BSSConnect(wdrvHandle, &bssCtx, &authCtx, &APP_ConnectNotifyCb);

                } else {
                    WDRV_WINC_BSSReconnect(wdrvHandle, &APP_ConnectNotifyCb);

                }
                WDRV_WINC_SystemTimeGetCurrent(wdrvHandle, &APP_GetTimeNotifyCb);

            }
#else
              if (wifi_mode == WIFI_DEFAULT)
            {
                /* Enable use of DHCP for network configuration, DHCP is the default
                but this also registers the callback for notifications. */
                WDRV_WINC_IPUseDHCPSet(wdrvHandle, &APP_DHCPAddressEventCb);

                debug_printGood("  APP: registering APP_CloudTaskcb");
                App_CloudTaskHandle = SYS_TIME_CallbackRegisterMS(APP_CloudTaskcb, 0, APP_CLOUDTASK_INTERVAL, SYS_TIME_PERIODIC);
                WDRV_WINC_BSSReconnect(wdrvHandle, &APP_ConnectNotifyCb);
                WDRV_WINC_SystemTimeGetCurrent(wdrvHandle, &APP_GetTimeNotifyCb);
            }          
#endif
            appData.state = APP_STATE_WDRV_ACTIV;
            break;
        }

        case APP_STATE_WDRV_ACTIV:
        {
            if (App_CloudTaskTmrExpired == true) {
                App_CloudTaskTmrExpired = false;
                CLOUD_task();
            }

            if (App_DataTaskTmrExpired == true) {
                App_DataTaskTmrExpired = false;
                APP_DataTask();
            }

            if (App_WifiScanPending) {
                APP_WifiScanTask(wdrvHandle);
            }

            CLOUD_sched();
            wifi_sched();
            MQTT_sched();
//            if (OSAL_QUEUE_Receive(&appData.appQueue, &appMsg, 10))
//            {
//                if (p_appMsg->msgId == APP_MSG_BLE_STACK_EVT)
//                {
//                    // Pass BLE Stack Event Message to User Application for handling
//                    APP_BleStackEvtHandler((STACK_Event_T *) p_appMsg->msgData);
//                } else if (p_appMsg->msgId == APP_MSG_BLE_STACK_LOG)
//                {
//                    // Pass BLE LOG Event Message to User Application for handling
//                    APP_BleStackLogHandler((BT_SYS_LogEvent_T *) p_appMsg->msgData);
//                } else if (p_appMsg->msgId == APP_MSG_UART_CB)
//                {
//                    // Pass BLE UART Data transmission target BLE UART Device handling
//                    APP_UartCBHandler();
//                }
//            }
            break;
        }
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

// This gets called by the scheduler approximately every 100ms

static void APP_DataTask(void) {
    // Get the current time. This uses the C standard library time functions
    time_t timeNow; // = time(NULL);
    struct tm sys_time;
    RTC_RTCCTimeGet(&sys_time);
    timeNow = mktime(&sys_time);
    // Example of how to send data when MQTT is connected every 1 second based on the system clock
    if (CLOUD_isConnected()) {
        // How many seconds since the last time this loop ran?
        int32_t delta = difftime(timeNow, previousTransmissionTime);
        
        timeNow = SYS_TIME_CounterGet();
        if (timeNow - previousTransmissionTime >= (telemetryInterval*SYS_TIME_FrequencyGet())) {
            previousTransmissionTime = timeNow;
            timeNow = mktime(&sys_time);
            // send telemetry
            APP_SendToCloud();
//            debug_printTrace("  LUC: Sending MQTT MESSAGE at %s", ctime(&timeNow));
//            static long count = 0;
//            uint8_t buffer[10];
//            sprintf(buffer,"%x",count++);
//            process_telemetry_command(2, buffer);
//            
        }

        check_button_status();

        if (shared_networking_params.reported == 0) {
            twin_properties_t twin_properties;

            init_twin_data(&twin_properties);

            strcpy(twin_properties.ip_address, deviceIpAddress);
            twin_properties.flag.ip_address_updated = 1;

            if (az_result_succeeded(send_reported_property(&twin_properties))) {
                shared_networking_params.reported = 1;
                debug_printInfo("  APP: Reported IP Address property");
            } else {
                debug_printError("  APP: Failed to report IP Address property");
            }
        }

        if (userdata_status.as_uint8 != 0) {
            uint8_t i;
            // received user data via UART
            // update reported property
            for (i = 0; i < 8; i++) {
                uint8_t tmp = userdata_status.as_uint8;
                if (tmp & (1 << i)) {
                    debug_printInfo("  APP: User Data Slot %d changed", i + 1);
                }
            }
            userdata_status.as_uint8 = 0;
        }
    } else {
        debug_printWarn("  APP: Not Connected to CLOUD");
    }
#ifdef _ELIMINATE
    if (shared_networking_params.haveAPConnection) {
        LED_SetBlue(LED_STATE_HOLD);
    } else {
        LED_SetBlue(LED_STATE_OFF);
    }

    if (shared_networking_params.haveERROR) {
        LED_SetWiFi(LED_INDICATOR_ERROR);
    } else {
        LED_SetWiFi(LED_INDICATOR_SUCCESS);
    }

    if (CLOUD_isConnected()) {
        LED_SetCloud(LED_INDICATOR_SUCCESS);
    } else {
        LED_SetCloud(LED_INDICATOR_ERROR);
    }
#endif /* _ELIMINATE */
}

// *****************************************************************************
// *****************************************************************************
// Section: Functions to interact with Azure IoT Hub and DPS
// *****************************************************************************
// *****************************************************************************

/**********************************************
 * Command (Direct Method)
 **********************************************/
void APP_ReceivedFromCloud_methods(uint8_t* topic, uint8_t* payload) {
    az_result rc;
#ifdef IOT_PLUG_AND_PLAY_MODEL_ID
    az_iot_pnp_client_command_request command_request;
#else
    az_iot_hub_client_method_request method_request;
#endif

    debug_printInfo("  APP: %s() Topic %s Payload %s", __FUNCTION__, topic, payload);

    if (topic == NULL) {
        debug_printError("  APP: Command topic empty");
        return;
    }

    az_span command_topic_span = az_span_create(topic, strlen((char*) topic));

#ifdef IOT_PLUG_AND_PLAY_MODEL_ID
    rc = az_iot_pnp_client_commands_parse_received_topic(&pnp_client, command_topic_span, &command_request);
#else
    rc = az_iot_hub_client_methods_parse_received_topic(&iothub_client, command_topic_span, &method_request);
#endif

    if (az_result_succeeded(rc)) {
        debug_printTrace("  APP: Command Topic  : %s", az_span_ptr(command_topic_span));
#ifdef IOT_PLUG_AND_PLAY_MODEL_ID
        debug_printTrace("  APP: Command Name   : %s", az_span_ptr(command_request.command_name));
#else
        debug_printTrace("  APP: Method Name   : %s", az_span_ptr(method_request.name));
#endif
        debug_printTrace("  APP: Command Payload: %s", (char*) payload);

#ifdef IOT_PLUG_AND_PLAY_MODEL_ID
        process_direct_method_command(payload, &command_request);
#else
        process_direct_method_command(payload, &method_request);
#endif
    } else {
        debug_printError("  APP: Command from unknown topic: '%s' return code 0x%08x.", az_span_ptr(command_topic_span), rc);
    }
}

/**********************************************
 * Properties (Device Twin)
 **********************************************/
void APP_ReceivedFromCloud_patch(uint8_t* topic, uint8_t* payload) {
    az_result rc;
    twin_properties_t twin_properties;

    init_twin_data(&twin_properties);

    twin_properties.flag.is_initial_get = 0;

    debug_printInfo("  APP: %s() Payload %s", __FUNCTION__, payload);

    if (az_result_failed(rc = process_device_twin_property(topic, payload, &twin_properties))) {
        // If the item can't be found, the desired temp might not be set so take no action
        debug_printError("  APP: Could not parse desired property, return code 0x%08x\n", rc);
    } else {
        if (twin_properties.flag.user_led_found == 1) {
            debug_printInfo("  APP: Found led_user = '%s' (%d)",
                    LED_Property[twin_properties.desired_led_user - 1],
                    twin_properties.desired_led_user);
        }
        if (twin_properties.flag.blue_led_found == 1) {
            debug_printInfo("  APP: Found rgb_led_blue = %d",
                    twin_properties.desired_led_blue);
        }
        if (twin_properties.flag.green_led_found == 1) {
            debug_printInfo("  APP: Found rgb_led_green = %d",
                    twin_properties.desired_led_green);
        }
        if (twin_properties.flag.red_led_found == 1) {
            debug_printInfo("  APP: Found rgb_led_red = %d",
                    twin_properties.desired_led_red);
        }
        if (twin_properties.flag.telemetry_interval_found == 1) {
            debug_printInfo("  APP: Found telemetryInterval value = %d", telemetryInterval);
        }

        update_leds(&twin_properties);
        rc = send_reported_property(&twin_properties);

        if (az_result_failed(rc)) {
            debug_printError("  APP: send_reported_property() failed 0x%08x", rc);
        }
    }
}

void APP_ReceivedFromCloud_twin(uint8_t* topic, uint8_t* payload) {
    az_result rc;
    twin_properties_t twin_properties;

    init_twin_data(&twin_properties);

    if (topic == NULL) {
        debug_printWarn("  APP: Twin topic empty");
        return;
    }

    debug_printTrace("  APP: %s() Payload %s", __FUNCTION__, payload);

    if (az_result_failed(rc = process_device_twin_property(topic, payload, &twin_properties))) {
        // If the item can't be found, the desired temp might not be set so take no action
        debug_printError("  APP: Could not parse desired property, return code 0x%08x\n", rc);
    } else {
        if (twin_properties.flag.is_initial_get) {
            iothubConnected = true;
        }
        if (twin_properties.flag.user_led_found == 1) {
            debug_printInfo("  APP: Found led_user = '%s' (%d)",
                    LED_Property[twin_properties.desired_led_user - 1],
                    twin_properties.desired_led_user);
        }
       if (twin_properties.flag.blue_led_found == 1) {
            debug_printInfo("  APP: Found rgb_led_blue = %d",
                    twin_properties.desired_led_blue);
        }
        if (twin_properties.flag.green_led_found == 1) {
            debug_printInfo("  APP: Found rgb_led_green = %d",
                    twin_properties.desired_led_green);
        }
        if (twin_properties.flag.red_led_found == 1) {
            debug_printInfo("  APP: Found rgb_led_red = %d",
                    twin_properties.desired_led_red);
        }
        if (twin_properties.flag.telemetry_interval_found == 1) {
            debug_printInfo("  APP: Found telemetryInterval = %d", telemetryInterval);
        }

        update_leds(&twin_properties);
        rc = send_reported_property(&twin_properties);

        if (az_result_failed(rc)) {
            debug_printError("  APP: send_reported_property() failed 0x%08x", rc);
        }
    }

    // debug_printInfo("  APP: << %s() rc = 0x%08x", __FUNCTION__, rc);
}

/**********************************************
 * Read Temperature Sensor value
 **********************************************/
float APP_GetTempSensorValue(void) {
    //float retVal = 0;
    float input_voltage = 0;
    
    uint16_t adc_count = 0;
    #define ADC_VREF                (3.3f)
    #define ADC_MAX_COUNT           (4095)
    ADCHS_GlobalEdgeConversionStart();

    /* Wait till ADC conversion result is available */
    while (!ADCHS_ChannelResultIsReady(ADCHS_CH2)) {
    };

    /* Read the ADC result */
    adc_count = ADCHS_ChannelResultGet(ADCHS_CH2);
    input_voltage = (float) adc_count * ADC_VREF / ADC_MAX_COUNT;
    float temperature;
    uint8_t buffer[30];

    temperature = (((input_voltage - (int) input_voltage)) - (float) 0.5) * 100;
    sprintf(buffer, " +TEMP:%.02f C", (float) temperature);
    BLE_TRSPS_SendData(conn_hdl, strlen(buffer), buffer);
    
    return temperature;
}

/**********************************************
 * Build light sensor value
 **********************************************/
/*
int32_t APP_GetLightSensorValue(void) {
    static uint64_t count; 
    return count++;
    return Button1_Get() ;
}
*/
/**********************************************
 * Entry point for telemetry
 **********************************************/
void APP_SendToCloud(void) {
    if (iothubConnected) {
        send_telemetry_message();
    }
}

/**********************************************
 * Callback functions for MQTT
 **********************************************/
void iot_connection_completed(void) {
    debug_printGood("  APP: %s()", __FUNCTION__);

    //LED_SetCloud(LED_INDICATOR_SUCCESS);

    App_DataTaskHandle = SYS_TIME_CallbackRegisterMS(APP_DataTaskcb, 0, APP_DATATASK_INTERVAL, SYS_TIME_PERIODIC);
}

#ifdef CFG_MQTT_PROVISIONING_HOST

void iot_provisioning_completed(void) {
    debug_printGood("  APP: %s()", __FUNCTION__);
    pf_mqtt_iothub_client.MQTT_CLIENT_task_completed = iot_connection_completed;
    CLOUD_init_host(hub_hostname, attDeviceID, &pf_mqtt_iothub_client);
    CLOUD_reset();
    //LED_SetCloud(LED_INDICATOR_PENDING);
}
#endif   // CFG_MQTT_PROVISIONING_HOST

void APP_WifiScan(char* buffer) {
    App_WifiScanPending = true;
    sprintf(buffer, "OK");
}

void APP_WifiGetStatus(char* buffer) {
    WDRV_WINC_SSID ssid;
    WDRV_WINC_STATUS status;

    status = WDRV_WINC_AssociationSSIDGet(wdrvHandle, &ssid, NULL);

    if (status == WDRV_WINC_STATUS_OK) {
        sprintf(buffer, "%s,%s", deviceIpAddress, ssid.name);
    } else {
        // debug_printError("  APP: WDRV_WINC_AssociationSSIDGet failed %d", status);
        sprintf(buffer, "0.0.0.0");
    }
}

static void APP_WifiScanTask(DRV_HANDLE handle) {
    WDRV_WINC_STATUS status;

    switch (appData.scanState) {
        case STATE_SCAN_INIT:

            memset(APP_WiFiApList, 0, sizeof (APP_WiFiApList));

            if (WDRV_WINC_STATUS_OK == WDRV_WINC_BSSFindFirst(wdrvHandle, WDRV_WINC_ALL_CHANNELS, true, NULL, NULL)) {
                appData.scanState = STATE_SCANNING;
            } else {
                appData.scanState = STATE_SCAN_ERROR;
            }
            break;

        case STATE_SCANNING:
            if (false == WDRV_WINC_BSSFindInProgress(handle)) {
                appData.scanState = STATE_SCAN_GET_RESULTS;
            }
            break;

        case STATE_SCAN_GET_RESULTS:
        {
            WDRV_WINC_BSS_INFO BSSInfo;
            if (WDRV_WINC_STATUS_OK == WDRV_WINC_BSSFindGetInfo(handle, &BSSInfo)) {
                status = WDRV_WINC_BSSFindNext(handle, NULL);
                if (WDRV_WINC_STATUS_BSS_FIND_END == status) {
                    appData.scanState = STATE_SCAN_DONE;
                    break;
                } else if (status != WDRV_WINC_STATUS_OK) {
                    appData.scanState = STATE_SCAN_ERROR;
                    break;
                } else if (BSSInfo.ctx.ssid.length > 0) {
                    if (strlen(APP_WiFiApList) > 0) {
                        strlcat(APP_WiFiApList, ",", sizeof (APP_WiFiApList) - 1);
                    }
                    strlcat(APP_WiFiApList, (const char*) &BSSInfo.ctx.ssid.name, sizeof (APP_WiFiApList) - 1);
                }
            }
            break;
        }

        case STATE_SCAN_DONE:
            App_WifiScanPending = false;
            strlcat(APP_WiFiApList, "\4", sizeof (APP_WiFiApList));
            debug_disable(true);
            SYS_CONSOLE_Message(0, APP_WiFiApList);
            debug_disable(false);
            appData.scanState = STATE_SCAN_INIT;
            break;

        case STATE_SCAN_ERROR:
            App_WifiScanPending = false;
            debug_printError("  APP: Scan failed");
            appData.scanState = STATE_SCAN_INIT;
            break;
    }
}

void HeartBeat_CallBack_On(uintptr_t context) {

    //USER_LED_Clear();
    SYS_TIME_CallbackRegisterMS((SYS_TIME_CALLBACK) HeartBeat_CallBack_Off, 0, 50 /*BLINK_RATE_MS*/, SYS_TIME_SINGLE);
    //LED_RED_Toggle();
}

void HeartBeat_CallBack_Off(uintptr_t context) {
    //USER_LED_Set();

    //    OLED_drawString(0, -1, (unsigned char *) "  ", TEXT_ALIGN_LEFT);

    SYS_TIME_CallbackRegisterMS((SYS_TIME_CALLBACK) HeartBeat_CallBack_On, 0, 950 /*BLINK_RATE_MS*/, SYS_TIME_SINGLE);

}
/*******************************************************************************
 End of File
 */
