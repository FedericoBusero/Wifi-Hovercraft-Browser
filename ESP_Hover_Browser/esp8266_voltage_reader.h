// Do not place this in the .ino file because the preprocessor cause compilation problems

// Necessary for reading voltage using ESP.getVcc
ADC_MODE(ADC_VCC); 

// change this voltage factor for your chip, every chip is different. Calibrate using USB power, which should result in 3.3V
#define VOLTAGE_FACTOR 910.0f 
