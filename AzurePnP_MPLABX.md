# Programming the Plug and Play Demo using Microchip's MPLAB X IDE

1. Connect the board to a USB port on the PC using a micro USB cable.

2. Launch the MPLAB X IDE and navigate to the main toolbar's `File` > `Open Project` operation to load the demo project folder (\*.X) located at `AzureDemo_WBX451` > `WBZ451_Curiosity` >`firmware` > `WBZ451_Trust_WiFi7.X`.

   If the `load error` message in red appears in the `Output` window, click on the `Resolve DFP for configuration: default` link

    <img src=".//media/image21.png" style="width:6.5in;height:1.00833in" alt="A screenshot of a cell phone Description automatically generated" />

3. Set the `WBZ451_Trust_WiFi7` project as the main (currently focused/active) project by right-clicking on it and selecting `Set as Main Project`.

    <img src=".//media/image41.png" style="width:5.in;height:3.18982in" alt="A screenshot of a cell phone Description automatically generated" />

4. Verify the project properties are set correctly before building the project. Right-click the project `WBZ451_Trust_WiFi7` &gt; select `Properties` &gt; select "WBZ451 Curiosity Board-SN" for `Connected Hardware Tool` &gt; select the latest `PIC32CX-BZ_DFP` version &gt; select the latest XC32 version for `Compiler Toolchain`. If any changes were made in the project properties window, the `Apply` button should become enabled.  Make sure to hit the `Apply` button before hitting `OK`.

5. Right-click the `WBZ451_TRUST_WiFi7` project and select `Make and Program Device`.  This operation will automatically clean and build the project before attempting to program the target device. After the `BUILD SUCCESSFUL` message appears in the Output window, the application HEX file will be programmed onto the WBZ451 Curiosity board. Once programming has finished, the board will automatically reset and start running its application code.
