// Niet verplaatsen naar .ino file want dit veroorzaakt preprocessor problemen

// Nodig voor het inlezen van het voltage met ESP.getVcc
ADC_MODE(ADC_VCC); 

// Pas de voltagefactor aan, dat is bij elke chip hetzelfde. Calibreer bv. met USB stroom die 3.3V op de chip moet geven
#define VOLTAGE_FACTOR 910.0f 
