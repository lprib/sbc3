#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

void Error_Handler(void);

#define DBG_LED_Pin GPIO_PIN_0
#define DBG_LED_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define CODEC_GPIO1_Pin GPIO_PIN_14
#define CODEC_GPIO1_GPIO_Port GPIOB
#define CODEC_NRESET_Pin GPIO_PIN_9
#define CODEC_NRESET_GPIO_Port GPIOA

void SystemClock_Config(void)
{
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

   __HAL_RCC_SYSCFG_CLK_ENABLE();

   /** Configure the main internal regulator output voltage
    */
   __HAL_RCC_PWR_CLK_ENABLE();
   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

   /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState = RCC_HSE_ON;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLM = 8;
   RCC_OscInitStruct.PLL.PLLN = 168;
   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
   RCC_OscInitStruct.PLL.PLLQ = 7;
   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
   {
      Error_Handler();
   }

   /** Initializes the CPU, AHB and APB buses clocks
    */
   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
   {
      Error_Handler();
   }
}

void write_leds(uint16_t val)
{
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);

   for (int i = 0; i < 16; i++)
   {
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, val & (0x8000 >> i) ? GPIO_PIN_SET : GPIO_PIN_RESET);
      // HAL_Delay(1);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
      // HAL_Delay(1);
      HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
   }
   // HAL_Delay(1);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
   // HAL_Delay(1);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

void task(void *thing)
{
   for (;;)
   {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
      vTaskDelay(pdMS_TO_TICKS(100));
   }
}

int main(void)
{
   HAL_Init();
   SystemClock_Config();

   __HAL_RCC_GPIOB_CLK_ENABLE();
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   GPIO_InitStruct.Pin = GPIO_PIN_0;
   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
   GPIO_InitStruct.Pin = BOOT1_Pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

   /* Ensure all priority bits are assigned as preemption priority bits. */
   // TODO(liam) what do, not in hal??
   // NVIC_PriorityGroupConfig(NVIC_PRIORITYGROUP_4);

   xTaskCreate(&task, "BLINK", 256, NULL, tskIDLE_PRIORITY + 1U, NULL);

   vTaskStartScheduler();

   while (1)
   {
   }
}

void Error_Handler(void)
{
   __disable_irq();
   while (1)
   {
   }
}

/** Handler for pure virtual function calls.  This should be included, otherwise
 * the exception handling is pulled in. */
void __cxa_pure_virtual()
{
   Error_Handler();
}