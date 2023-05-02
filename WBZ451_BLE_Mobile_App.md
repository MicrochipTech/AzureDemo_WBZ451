# BLE Mobile App for the WBZ451 Curiosity Development Board

<img src=".//media/BLE-2-Cloud_Bridge.png" width=600/>

## Installing the BLE Mobile App

1. Locate the APK (Android Package Kit) file (e.g. "WBZ451_TRUST_WiFi7.apk") in the [mobile_app](./mobile_app/) folder.

2. Transfer the APK file to your smartphone. For example, you could use Google drive to copy the APK file to the cloud and then access it from the smartphone.

3. Install the mobile app by opening the APK file from your smartphone. The name of the app that should be installed is `WBZAzure`.

## Running the BLE Mobile App

1. Launch the `WBZAzure` app from your smartphone.

2. Touch the `Start Scan` button.

3. The app will start building a list of all the addresses of BLE devices that are currently in range and advertising mode. You should see one address that includes the string `WBZ451-UART-WINC` which belongs to your device.

4. Touch the `CONNECT` button and then touch the address corresponding to your device.

5. Upon successful connection, the app will display the `"Connected.."` message along with a picture of the WBZ451 Curiosity board and the current temperature reading at the top of the screen.

6. Touch the `LED Red` button and verify that the RGB LED turns on as red. Touch the `LED Red` button again and verify that the RGB LED turns off.

7. Repeat the RGB LED on/off test using the `LED Green` and `LED Blue` buttons.

8. Access the cloud application from the [IoT Central portal](https://apps.azureiotcentral.com). Using the left-hand navigation pane, navigate to `Connect` > `Devices` and click on your device name.

9. In the mobile app, type in a text message under `Telemetry Message String (ASCII)` and then touch the `Send Message` button.

10. In the cloud app, click on the `Raw Data` tab and verify that the telemetry message was received and the correct message is displayed under the `Message Event` column.

    <img src=".//media/ble_run_10.png" width=800/>

11. In the cloud app, click on the `Commands` tab. Using the `Echo Message` command box, type in a text message for `String to echo`. Click the `Run` button.

    <img src=".//media/ble_run_11.png" width=400/>

12. In the mobile app, verify that the text message is displayed under `Echo Message String Received (ASCII)`.

13. Touch the `DISCONNECT` button to properly disconnect from the WBZ451 Curiosity board.

## Building the BLE Mobile App

1. The MIT App Inventor is a web-based tool used for creating mobile apps for Android devices. Go to the [MIT App Inventor](https://appinventor.mit.edu) web site and click on the `Create Apps!` button at the top of the page.

2. Using a File Explorer (or equivalent) window, navigate to the [mobile_app](./mobile_app/) folder. Copy the `WBZ451_TRUST_WiFi7.aia` file and rename it.

2. Using the main toolbar at the top of the page, click on the Projects drop-down menu and select `Import project (.aia) from my computer`.

    <img src=".//media/ble_mod_03.png" width=500/>

3. Click on `Choose File` and browse to the newly-created *.aia file in the [mobile_app](./mobile_app/) folder. Click on the `Upload` button and then `OK`.

4. Let's try building the project without making any modifications. Using the main toolbar at the top of the page, click on the `Build` drop-down menu and select `Android App (.apk)`. Wait for the compilation process to finish; there should be no errors.

5. Click on the `Download .apk now` button and keep the file in a safe place. Try scanning the bar code to see if you are able to successfully install this version of the mobile app which was just built.

6. Using the main toolbar at the top of the page, click on `Guide` to learn all the details on how to configure mobile apps using the MIT App Inventor web-based tool.

    <img src=".//media/ble_mod_07.png" width=500/>

## References - Additional Resources

- [MIT App Inventor Library: Documentation & Support](https://appinventor.mit.edu/explore/library)

- [Microchip University](https://mu.microchip.com): [Rapid Prototyping Bluetooth Low Energy (BLE) Android Apps using MIT App Inventor](https://mu.microchip.com/rapid-prototyping-bluetooth-low-energy-ble-android-apps-using-mit-app-inventor)

    About this course: This class will give the engineer a hands-on introduction to Android App development using MIT App Inventor. The material explained in this class will cover the steps required to develop an Android App that interacts with the AVR-BLE board. Prior knowledge of the basic Bluetooth® Low Energy (BLE) protocol is required and will not be covered in this class. Covered topics include:  BLE scanning, BLE connection, BLE services, BLE characteristics, UUID’s and MAC addresses.

