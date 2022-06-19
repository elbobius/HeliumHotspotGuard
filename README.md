# HeliumHotspotGuard
 This is a project for an esp8266 based wifi plug to restart your helium miner if it hasn't done anything for a configurable amount of blocks.  
 After the first startup or if the button is pressed, it starts up as an access point to configure ssid and passord for the wifi connection.  
 If you after it is connected to your wifi enter the ip of the device(you get the ip propably from your router or dhcp server) the name should be somthing like "ESP-153344".  
   
This is how the webinterface looks like:  
![image](https://user-images.githubusercontent.com/84288274/174502670-345026aa-2d6d-44ce-99d1-3079db3ca103.png)  
 Internet connection is needed, because it checks the latest blockactivity from the configured minrer via api.helium.io  
   
 I have tested it with Sonoff r2, but it can work with every esp8266 based wifi plug some changes in the code.  

Code is still under construction but should work.
