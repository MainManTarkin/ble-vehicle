#include "vehicleControl.h"

#define hornPin 2

#define blueSPin 23
#define redSPin 22
#define greenSPin 32

#define rightMotor 17
#define leftMotor 5

#define rigthMotorDirPin1 19
#define rightMotorDirpin2 18

#define leftMotorDirPin1 15
#define leftMotorDirpin2 4

//CONNECT PINS AS GRB

volatile u_int32_t rbgValue = 0;
volatile u_int8_t hornValue = 0;
volatile unsigned long motorControlValue[2] = {0,0};
TaskHandle_t vehicalEventHandle = NULL;
SemaphoreHandle_t xMutex = NULL;

volatile static char rgbValueChange = 0;
volatile static char motorValueChange = 0;
volatile static char hornValueChange = 0;

volatile static char eventInited = 0;

bool vehicleDisconnectFlag = false;

static char pwmA = 0;
static char pwmB = 0;

static long countVal = 1;

volatile static long maxCountValA = 0;
volatile static long maxCountValB = 0;

volatile static uint8_t  ledPwmCount = 1;
volatile static uint8_t maxRedCount = 0xff;
volatile static uint8_t maxGreenCount = 0xff;
volatile static uint8_t maxBlueCount = 0xff;
volatile static uint8_t maxAlphaCount = 0xff;

static bool IRAM_ATTR example_timer_on_alarm_cb_v1(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
    
    //handle motor pwm
    if(countVal > 100){
		
		countVal = 1;
	}
	
	
	
	if(countVal <= maxCountValA){
		
		pwmA = 1;
		
		
	}else{
		
		pwmA = 0;

		
	}
	
	if(countVal <= maxCountValB){
		
		pwmB = 1;
		
		
	}else{
		
		pwmB = 0;

		
	}
	
	
	gpio_set_level(rightMotor, pwmA);
	gpio_set_level(leftMotor, pwmB);
	
	//handle headlight pwm
	
	if(ledPwmCount > maxAlphaCount){
		
		ledPwmCount = 1;
	}
	
	if(ledPwmCount <= maxRedCount){
		
		gpio_set_level(redSPin, 1);
		
		
	}else{
		
		gpio_set_level(redSPin, 0);

		
	}
	
	if(ledPwmCount <= maxGreenCount){
		
		gpio_set_level(greenSPin, 1);
		
		
	}else{
		
		gpio_set_level(greenSPin, 0);

		
	}
	
	if(ledPwmCount <= maxBlueCount){
		
		gpio_set_level(blueSPin, 1);
		
		
	}else{
		
		gpio_set_level(blueSPin, 0);

		
	}
	
	countVal++;
	ledPwmCount++;
    // return whether we need to yield at the end of ISR
    return (high_task_awoken == pdTRUE);
}


void initVehicleControl()
{

    // init the eventList
    xMutex = xSemaphoreCreateMutex();
   
	
	//setup headlight pwm
	gpio_set_direction(blueSPin, GPIO_MODE_OUTPUT);
	gpio_set_direction(redSPin, GPIO_MODE_OUTPUT);
	gpio_set_direction(greenSPin, GPIO_MODE_OUTPUT);
	
	gpio_set_level(blueSPin, 1);
	gpio_set_level(redSPin, 1);
	gpio_set_level(greenSPin, 1);
	
	
	
	//horn gpio setup
	 gpio_set_direction(hornPin, GPIO_MODE_OUTPUT);
    gpio_set_level(hornPin, 0);
    
	
	//motor pwm setup
	gpio_set_direction(rightMotor, GPIO_MODE_OUTPUT);
	gpio_set_direction(leftMotor, GPIO_MODE_OUTPUT);
	gpio_set_direction(rigthMotorDirPin1, GPIO_MODE_OUTPUT);
	gpio_set_direction(rightMotorDirpin2, GPIO_MODE_OUTPUT);
	
	gpio_set_level(rightMotor, 0);
	gpio_set_level(leftMotor, 0);
	
	gpio_set_level(rigthMotorDirPin1, 1);
	gpio_set_level(rightMotorDirpin2, 0);
	
	gpio_set_level(leftMotorDirPin1, 1);
	gpio_set_level(leftMotorDirpin2, 0);
	
	
	ESP_LOGI(vehicalLog, "Create timer handle");
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 40000000, // 1MHz, 1 tick=1us
    };
	
	ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = example_timer_on_alarm_cb_v1,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    ESP_LOGI(vehicalLog, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
	
	ESP_LOGI(vehicalLog, "Start timer, stop it at alarm event");
    gptimer_alarm_config_t alarm_config1 = {
		.reload_count = 0,
        .alarm_count = 1000, // period = 1s
		.flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
   

	eventInited = 1;
}

void vehicalValUpdate(enum vehicalEventTypes eventType)
{
    if (xSemaphoreTake(xMutex, (5 / portTICK_PERIOD_MS)))
    {
        switch (eventType)
        {
        case motorValChange:
            motorValueChange = 1;
            break;
        case rgbValChange:
            rgbValueChange = 1;
            break;
        case hornValChange:
            hornValueChange = 1;
            break;

        default:
            ESP_LOGI(vehicalLog, "invaild event type used in vehicalValUpdate()\n");
            break;
        }
        xSemaphoreGive(xMutex);
    }
}

static void hornStateChange(uint8_t hornState){

    if(hornState){

        gpio_set_level(hornPin, 1);

    }else{

        gpio_set_level(hornPin, 0);

    }


}

static void motorDirChange(int motorA1, int motorA2,long motorVal){
	
	if(motorVal < 0){
		
		gpio_set_level(motorA1, 0);
		gpio_set_level(motorA2, 1);
		
	}else{
		
		gpio_set_level(motorA1, 1);
		gpio_set_level(motorA2, 0);
		
	}
	
}

static long correctMotorVal(long motorVal){
	
	long motorValReturn = 0;
	
	if(motorVal > 100){
		
		motorValReturn = 100;
		
	}else{
		
		motorValReturn = motorVal;
		
	}
	
	if(motorVal < 0){
	
		if(motorVal < -100){
		
			motorValReturn = 100;
		
		}else{
			
			motorValReturn = motorVal * -1;
			
		}
	
	
	}
	
	
	
	return motorValReturn;
	
}

void vehicalEventLoop()
{
	
	long motorValA = 0;
	long motorValB = 0;

    uint8_t redVal = 0;
    uint8_t greenVal = 0;
    uint8_t blueVal = 0;
	uint8_t alphaVal = 0;
	
    if (!eventInited)
    {

        ESP_LOGI(vehicalLog, "vehicalEventLoop failed to run since vehicaleventinit was never called\n");
        vTaskDelete(vehicalEventHandle);
    }

    ESP_LOGI(vehicalLog, "event task created \n");
    while (1)
    { // taskLoop

        if (xSemaphoreTake(xMutex, (5 / portTICK_PERIOD_MS)))
        {

            if (rgbValueChange)
            {

                if (rgbValueChange)
            {

				
				redVal = (uint8_t)((rbgValue & 0xff000000) >> 24);
                greenVal = (uint8_t)((rbgValue & 0x00ff0000) >> 16);
                blueVal = (uint8_t)((rbgValue & 0x0000ff00) >> 8);
				alphaVal = (uint8_t)(rbgValue & 0x000000ff);
				
				ESP_LOGI(vehicalLog, "changed red value: %d\n", redVal);
                ESP_LOGI(vehicalLog, "changed green value: %d\n", greenVal);
                ESP_LOGI(vehicalLog, "changed blue value: %d\n", blueVal);
				ESP_LOGI(vehicalLog, "changed alpha value: %d\n", alphaVal);

				maxRedCount = redVal;
                maxGreenCount = greenVal;
                maxBlueCount = blueVal;
				maxAlphaCount = alphaVal;
				
				
				rgbValueChange = false;
            }else{

                ESP_LOGI(vehicalLog, "rgb value was not written to");
                
            }
                rgbValueChange = 0;
            }else{

                ESP_LOGI(vehicalLog, "rgb value was not written to");
                
            }

            if (motorValueChange)
            {

                ESP_LOGI(vehicalLog, "changed motorValue 1 %d\n", motorControlValue[0]);
                ESP_LOGI(vehicalLog, "changed motorValue 2 %d\n", motorControlValue[1]);
				
				
				
				motorValA = motorControlValue[0];
				motorValB = motorControlValue[1];
				
				
				
				motorDirChange(rigthMotorDirPin1, rightMotorDirpin2, motorValA);
				motorDirChange(leftMotorDirPin1, leftMotorDirpin2, motorValB);
				
				motorValA = correctMotorVal(motorValA);
				motorValB = correctMotorVal(motorValB);
				
				
				
				maxCountValA = motorValA;
				maxCountValB = motorValB;
				
				ESP_LOGI(vehicalLog, "new maxCountValA: %d", (int)maxCountValA);
				ESP_LOGI(vehicalLog, "new maxCountValB: %d", (int)maxCountValB);
				
                motorValueChange = 0;
            }else{

                ESP_LOGI(vehicalLog, "motor value was not written to");
                
            }

            if (hornValueChange)
            {

                hornStateChange(hornValue);
                ESP_LOGI(vehicalLog, "hornValue changed %d\n", hornValue);
                hornValueChange = 0;
            }else{

                ESP_LOGI(vehicalLog, "horn value was not written to");

            }

            xSemaphoreGive(xMutex);
        }else{


            ESP_LOGE(vehicalLog, "failed to aquire mutex");

        }
		
        


		if(vehicleDisconnectFlag){
			
			
			maxCountValA = 0;
			maxCountValB = 0;
			hornValue = 0;
			rbgValue = 0;
			maxRedCount = 0;
			maxGreenCount = 0;
			maxBlueCount = 0;
			maxAlphaCount = 255;
			hornStateChange(hornValue);
			maxRedCount = 0xff;
			maxGreenCount = 0xff;
			maxBlueCount = 0xff;
			maxAlphaCount = 0xff;
			motorDirChange(rigthMotorDirPin1,rightMotorDirpin2, 1);
			motorDirChange(leftMotorDirPin1,leftMotorDirpin2, 1);
			
			vehicleDisconnectFlag = false;
		}
		
		
        vTaskDelay(700 / portTICK_RATE_MS);
    }
}