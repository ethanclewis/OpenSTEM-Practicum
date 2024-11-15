# Engine
All scripts collaborate to automate the OpenSTEM prototype unit's data uploading and environmental condition regulation.  
All scripts are managed and run by the Raspberry Pi's command-line job scheduler: cron.

### *DHT_upload.py*
Retrieves readings from all three DHT sensors, and uploads data to the respective InfluxDB bucket/ measurement.      
Successful data uploads and errors are logged in a .txt file.      
**Execute every 30 minutes with cron (\30 \* \* \* \*)** 

### *water_main.py*
Works in tandem with *wired_to_pi_v5.ino* to...    
- Dictate the reading of RTD, pH, and EC sensors, and uploads this data to the respective InfluxDB bucket/ measurement.  
- Dictate the execution of pump control logic to regulate pH and EC levels.
      
Successful data uploads and errors are logged in a .txt file.      
**Execute every 30 minutes with cron (\30 \* \* \* \*)**

### *wired_to_pi_v5.ino*
Works in tandem with *water_main.py* to receive the follwing commands...    
- **"R":** Read the RTD, pH, and EC sensors and pass these values to *water_main.py*.  
- **"P":** Read the pH and EC sensors and dictate pump control logic based on their values.
  
**Runs indefinitely once upload to Arduino; no need for cron scheduling** 


# Lights
### *automated_lights_seeds.py*
Automates the control of all system grow lights based on current time of day during seedling/ germination period.  
- Lights On from 8AM - 12PM
  
**Execute every 1 hour with cron (0 \* \* \* \*)**

### *automated_lights.py*
Automates the control of all system grow lights based on current time of day during adulthood.  
- Lights On from 8AM - 8PM
  
**Execute every 1 hour with cron (0 \* \* \* \*)**

### *manual_light_switch.py*
Provides a command-line interface to toggle all system grow lights.    
Lights remain in most recent power state after the script is terminated.
