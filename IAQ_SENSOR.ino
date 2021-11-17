#include "bsec.h"

#define LED_GREEN 35
#define LED_BLUE 36
#define IAQ_ACCURACY_THRESHOLD 1 // 0 => sensor calibration in progress : else => sensor ready
#define IAQ_INDEX 0
#define VOC_INDEX 1
#define SENSOR_DEFAULT 0


Bsec iaqSensor;                                                       // Create an instance of Bsec (BME680 sensor)
String output;                                                       // string to print on serial port
uint16_t air_quality_metrics[2] = {SENSOR_DEFAULT, SENSOR_DEFAULT}; // bluetooth payload data
bool updated_payload_available;                                    // flag to poll for monitoring updated payload 0 => no new data available : else => new data available for transmission

void sensor_config();
void update_payload();

// Initialization
void setup(void)
{
  Serial.begin(115200);
  Wire.begin();

  while(!Serial){
  };

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);

  sensor_config();

  // Print sensor output parameters
  output = "System metrics: IAQ, VOC";
  Serial.println(output);
}

// While(1) function
void loop(void)
{
  if(iaqSensor.status == BSEC_OK){    
      if (iaqSensor.run()) { // If new data is available
        digitalWrite(LED_GREEN, HIGH);
        update_payload();
        output = "IAQ: " + String(iaqSensor.iaq);
        output += ", IAQAccuracy: " + String(iaqSensor.iaqAccuracy); 
        output += ", VOC: " + String(iaqSensor.breathVocEquivalent);
        Serial.println(output);
      }
  }  
  else{ // Sensor failure condition
    output = "BSEC error code : " + String(iaqSensor.status);
    Serial.println(output);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, HIGH);
  }
}

void sensor_config(){
  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);

  bsec_virtual_sensor_t sensorList[2] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
  };

  iaqSensor.updateSubscription(sensorList, 2, BSEC_SAMPLE_RATE_ULP);
}

void update_payload(){
   updated_payload_available = false;                                                                                           
   if (iaqSensor.iaqAccuracy >= IAQ_ACCURACY_THRESHOLD){                                                                       //check for sensor calibration complettion
      if(iaqSensor.iaq != air_quality_metrics[IAQ_INDEX] || iaqSensor.breathVocEquivalent != air_quality_metrics[VOC_INDEX]){ // check if the current data is same as previous data
          updated_payload_available = true;
          air_quality_metrics[IAQ_INDEX] = iaqSensor.iaq;
          air_quality_metrics[VOC_INDEX] = iaqSensor.breathVocEquivalent;
      }
   }
}
