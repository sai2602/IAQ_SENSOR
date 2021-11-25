#include "bsec.h"

#define LED_GREEN 35                                             // Green LED pin number
#define LED_BLUE 36                                             // Blue LED pin number
#define IAQ_ACCURACY_THRESHOLD 3                               // 0,1,2 => sensor calibration in progress : else => sensor ready


Bsec iaqSensor;                                             // Create an instance of Bsec (BME680 sensor)
String output;                                             // string to print on serial port

typedef enum print_log{
  SENSOR_CONFIG,
  SENSOR_FAILURE,
  SYSTEM_METRICS,
  SENSOR_DATA  
}log_info;


typedef enum led_config{
  INIT,
  SENSOR_RUNNING,
  SENSOR_FAILURE_LED
}led_state;


struct payload{
  int8_t temperature;                                 // Indoor temperature
  float humidity;                                    // Indoor humidity
  uint16_t iaq;                                     // Indoor Air Quality
  float voc;                                       // Volatile Organic Compound (Gas metric)
  bool valid_iaq;                                 // False => IAQ value unstable : else IAQ value stable
}air_quality_metrics;


void sensor_config();                           // Initialize BME680 sensor
void update_payload();                         // Update payload values
void update_led(led_state current_led_state); // Update LED status
void print_log_info(log_info log_data);      // Print log data on serial monitor

// Initialization
void setup(void)
{
  Serial.begin(115200);
  Wire.begin();

  while(!Serial){
  };

  update_led(INIT);
  
  sensor_config();

  print_log_info(SYSTEM_METRICS);
  
}

// While(1) function
void loop(void)
{
  if(iaqSensor.status == BSEC_OK){    
      if (iaqSensor.run()) { // If new data is available
        update_led(SENSOR_RUNNING);
        
        update_payload();
        
        print_log_info(SENSOR_DATA);
      }
  }  
  else{ // Sensor failure condition
    print_log_info(SENSOR_FAILURE);

    update_led(SENSOR_FAILURE_LED);
  }
}

void sensor_config(){
  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);

  print_log_info(SENSOR_CONFIG);
  
  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
}

void update_payload(){
   air_quality_metrics.temperature = iaqSensor.temperature;
   air_quality_metrics.humidity = iaqSensor.humidity;
   air_quality_metrics.iaq = iaqSensor.iaq;
   air_quality_metrics.voc = iaqSensor.breathVocEquivalent;                                                                                 
   air_quality_metrics.valid_iaq = iaqSensor.iaqAccuracy >= IAQ_ACCURACY_THRESHOLD ? true : false;   
}

void update_led(led_state current_led_state){
  switch(current_led_state){
    case INIT:
      pinMode(LED_GREEN, OUTPUT);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, LOW);
     break;
    case SENSOR_RUNNING:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, LOW);
     break;
    case SENSOR_FAILURE_LED:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, HIGH);
     break;
    default:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, LOW);
  }
}

void print_log_info(log_info log_data){
  switch(log_data){
    case SENSOR_CONFIG:
      output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
      Serial.println(output);
    break;
    case SENSOR_FAILURE:
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
    break;
    case SYSTEM_METRICS:
      output = "System metrics: Temperature, Humidity, IAQ, VOC";
      Serial.println(output);
    break;
    case SENSOR_DATA:
      output = "Temperature: " + String(iaqSensor.temperature);
      output += ", Humidity: " + String(iaqSensor.humidity);
      output += ", IAQ: " + String(iaqSensor.iaq);
      output += ", IAQAccuracy: " + String(iaqSensor.iaqAccuracy); 
      output += ", VOC: " + String(iaqSensor.breathVocEquivalent);
      Serial.println(output);
    break;
    default:
      output = "No valid log data";
      Serial.println(output);
  }
}
