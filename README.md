Bochra System - ESP32 Smart Greenhouse Monitoring & Control System
Bochra System is an ESP32-based embedded system programmed in bare-metal C++ for automated greenhouse microclimate monitoring and control. The system regulates humidity, temperature, light intensity, and water levels in real-time to ensure optimal growth conditions while protecting hardware components from operational failures.
 Key Features
• Water Level Monitoring & Dry-Run Protection: Uses an ultrasonic sensor to monitor tank capacity. Automatically cuts off the pump relay and triggers an alert if the water level drops below 15% to prevent mechanical pump damage.
• Closed-Loop Climate Automation: Reads environmental metrics via a DHT22 sensor to dynamically control a sprayer pump (humidity management) and an exhaust fan (temperature control).
• Light Intensity Sensing: Measures light levels in Lux using a photoresistor (LDR) with a custom transfer function and raises visual/audible alarms if ambient light exceeds pre-configured thresholds.
• IoT Telemetry & Remote Control: Streams real-time telemetry metrics and status flags to a cloud dashboard.
• Local User Interface & Diagnostics: Displays status messages and sensor data locally on an I2C LCD1602 screen and dual-LED indicators.
 Hardware Architecture & Components
• Microcontroller: ESP32 Board
• Sensors: DHT22 (Temperature & Relative Humidity)
HC-SR04 Ultrasonic Sensor (Water Level Detection)
LDR Module (Light Intensity / Lux Calculation)
• Actuators & Outputs: 2-Channel Relay Module (Sprayer Pump & Exhaust Fan)
I2C LCD 16x2 Display
Active Buzzer (Audible Alarm)
Status LEDs (Green: Safe System Status / Red: Warning Triggered)
 System Logic & Thresholds
Parameter	Sensor	Safe Range / Condition	Action Triggered
Water Level	HC-SR04	> 15%	Forced Pump Cutoff if < 15%
Humidity	DHT22	70% - 80%	Sprayer ON if < 70%, OFF if >= 80%
Temperature	DHT22	≤ 28°C	Exhaust Fan ON if > 28°C
Light Level	LDR	≤ 50 Lux	Warning Alarm if > 50 Lux

