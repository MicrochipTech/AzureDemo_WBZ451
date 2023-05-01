# BLE Mobile App for the WBZ451 Curiosity Development Board

## Setting up the BLE Mobile App

1. Locate the APK (Android Package Kit) file (e.g. "WBZ451_BLE_Azure.apk") in the [Mobile_App](./Mobile_App/) folder.

2. Transfer the APK file to your smartphone. For example, you could use Google drive to copy the APK file to the cloud and then access it from the smartphone.

3. Install the mobile app by opening the APK file from your smartphone. The name of the app should be installed as `WBZAzure`.

## Running the BLE Mobile App

1. Launch the `WBZAzure` app from your smartphone.

2. Touch the `Start Scan` button.

3. The app will start building a list of all the addresses of BLE devices that are currently in range and advertising themselves. You should see an address that includes the string `WBZ451-UART-WINC` which belongs to your device.

4. Touch the `Connect to..` button and then touch the address corresponding to your device.

5. Upon successful connection, the app will display the `"Connected.."` message along with a picture of the WBZ451 Curiosity board and the current temperature reading at the top of the screen.

6. Touch the `LED Red` button and verify that the RGB LED turns on as red. Touch the `LED Red` button again and verify that the RGB LED turns off.

7. Repeat the RGB LED on/off test using the `LED_Green` and `LED_Blue` buttons.

8. Access the cloud application from the [IoT Central portal](https://apps.azureiotcentral.com). Using the left-hand navigation pane, navigate to `Connect` > `Devices` and click on your device name.

9. In the mobile app, type in a text message under `Telemetry Message String (ASCII)` and then touch the `Send Message` button.

10. In the cloud app, click on the `Raw Data` tab and verify that the telemetry message was received.

11. In the cloud app, click on the `Commands` tab. Using the `Echo Message` command box, type in a text message for `String to echo`. Click the `Run` button.

12. In the mobile app, verify that the text message is displayed under `Echo Message String (ASCII)`.

13. Touch the `Disconnect` button to properly disconnect from the WBZ451 Curiosity board.
