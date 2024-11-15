#include <Wire.h>
#include <Ezo_i2c.h>

// Define the sensors and pumps with I2C addresses
Ezo_board PH = Ezo_board(99, "PH");
Ezo_board EC = Ezo_board(100, "EC");
Ezo_board RTD = Ezo_board(102, "RTD");
Ezo_board PMP1 = Ezo_board(56, "PMP1");  // pH up
Ezo_board PMP2 = Ezo_board(57, "PMP2");  // pH down
Ezo_board PMP3 = Ezo_board(58, "PMP3");  // Nutrients

// Thresholds for pump control
float PH_LOWER_LIMIT = 5.5;
float PH_UPPER_LIMIT = 6.5;
float EC_LOWER_LIMIT = 1000.0;
float EC_UPPER_LIMIT = 2000.0;
// Dose volume
float PUMP_DOSE = -0.5;

// Establish wired connection
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Arduino Ready");
}


// MAIN LOOP
void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'R') {  // 'R' = Read Sensors
      poll_sensors();
    }
    else if (command == 'P') {  // 'P' = Execute Pump Control Logic
      control_pumps();
    }
  }
  delay(100);  // Small delay for stability
}

// Function to read  sensor values
void poll_sensors() {
  float ph_value = read_ph();
  float ec_value = read_ec();
  float temp_value = read_temp();

// water_main.py retrieves sensor readings from these printed values
  Serial.print("pH: "); Serial.println(ph_value);
  Serial.print("EC: "); Serial.println(ec_value);
  Serial.print("Temperature: "); Serial.println(temp_value);
}

// Function to construct pump control logic
void control_pumps() {
  // Read necessary sensor values
  float ph_value = read_ph();
  float ec_value = read_ec();

  // pH control logic
  if (ph_value < PH_LOWER_LIMIT) {
    Serial.println("pH below limit. Triggering PMP1...");
    PMP1.send_cmd_with_num("d,", PUMP_DOSE);  // Dispense pH up
  } else if (ph_value > PH_UPPER_LIMIT) {
    Serial.println("pH above limit. Triggering PMP2...");
    PMP2.send_cmd_with_num("d,", PUMP_DOSE);  // Dispense pH down
  } else {
    Serial.println("pH is within range.");  // Do nothing
  }

  // EC control logic
  if (ec_value < EC_LOWER_LIMIT) {
    Serial.println("EC below limit. Triggering PMP3...");
    PMP3.send_cmd_with_num("d,", PUMP_DOSE);  // Dispense nutrients
  } else if (ec_value > EC_UPPER_LIMIT) {
    Serial.println("EC above limit. Notification only.");
  } else {
    Serial.println("EC is within range.");  // Do nothing
  }
}

// Function to read pH sensor
float read_ph() {
  char sensordata_buffer[32];
  PH.send_read_cmd();
  delay(1000);  // Wait for sensor response
  if (PH.receive_cmd(sensordata_buffer, sizeof(sensordata_buffer)) == Ezo_board::SUCCESS) {
    return String(sensordata_buffer).toFloat();
  } else {
    Serial.println("Error reading pH sensor.");
    return -1;
  }
}

// Function to read EC sensor
float read_ec() {
  char sensordata_buffer[32];
  EC.send_read_cmd();
  delay(1000);
  if (EC.receive_cmd(sensordata_buffer, sizeof(sensordata_buffer)) == Ezo_board::SUCCESS) {
    return String(sensordata_buffer).toFloat();
  } else {
    Serial.println("Error reading EC sensor.");
    return -1;
  }
}

// Function to read Temperature (RTD) sensor
float read_temp() {
  char sensordata_buffer[32];
  RTD.send_read_cmd();
  delay(1000);
  if (RTD.receive_cmd(sensordata_buffer, sizeof(sensordata_buffer)) == Ezo_board::SUCCESS) {
    return String(sensordata_buffer).toFloat();
  } else {
    Serial.println("Error reading Temperature sensor.");
    return -1;
  }
}