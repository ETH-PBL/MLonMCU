#include "app.h"
#include "usart.h"


#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "q8fmnist.h"
#include "data.h"

extern "C" {
	#include "CycleCounter.h"
}

tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* nn = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* nn_input = nullptr;
TfLiteTensor* nn_output = nullptr;
constexpr size_t kTensorArenaSize = 60*1024;
uint8_t tensor_arena[kTensorArenaSize];


int application(void)
{
	// Setup code
//	MicroPrintf("Hello there");
	TfLiteStatus tflite_status;
	tflite::InitializeTarget();
	// Set up logging (modify tensorflow/lite/micro/debug_log.cc)
	static tflite::MicroErrorReporter micro_error_reporter;
	error_reporter = &micro_error_reporter;

	//   Map the model into a usable data structure
	nn = tflite::GetModel(q8fmnist);
	if (nn->version() != TFLITE_SCHEMA_VERSION)
	{
		error_reporter->Report("Model version does not match Schema\n");
	}

	static tflite::AllOpsResolver micro_op_resolver;

	static tflite::MicroAllocator *micro_allocator;
	micro_allocator = tflite::MicroAllocator::Create(tensor_arena, kTensorArenaSize, error_reporter);
	// Build an interpreter to run the model with.
	static tflite::MicroInterpreter static_interpreter(
			nn, micro_op_resolver, micro_allocator, error_reporter);
	interpreter = &static_interpreter;

	// Allocate memory from the tensor_arena for the model's tensors.
	tflite_status = interpreter->AllocateTensors();
	if (tflite_status != kTfLiteOk){
		error_reporter->Report("AllocateTensors() failed\n");
	}

	TfLiteTensor* nn_input = interpreter->input(0);
	TfLiteTensor* nn_output = interpreter->output(0);


	while(1){

		/*
		 * To measure the inference cycles, comment out the GUI CODE
		 * and uncomment the CYCLE MEASUREMENT code. Otherwise the GUI
		 * will get confused by the printed cycle counts.
		 */

		// GUI CODE BEGIN 1
		HAL_UART_Receive(&huart1, (uint8_t*)nn_input->data.int8, 28*28*1, HAL_MAX_DELAY);
		HAL_Delay(1000);
		HAL_UART_Transmit(&huart1, (uint8_t*)nn_input->data.int8, 28*28*1, HAL_MAX_DELAY);
		HAL_Delay(1000);
		// GUI CODE END 1

		// CYCLE MEASUREMENT CODE BEGIN 1
//		nn_input->data.int8 = (int8_t*)test_image;
//		ResetTimer();
//		StartTimer();
		// CYCLE MEASUREMENT CODE END 1

		// INFERENCE
		tflite_status = interpreter->Invoke();

		// CYCLE MEASUREMENT CODE BEGIN 2
//		StopTimer();
//		unsigned int cycles = getCycles();
//		printf("CPU Cycles: %u\r\n", cycles);
		// CYCLE MEASUREMENT CODE END 2

		// GUI CODE BEGIN 2
		if(tflite_status != kTfLiteOk){
			error_reporter->Report("Invoke failed");
		}
		HAL_UART_Transmit(&huart1, (uint8_t*)nn_output->data.int8, 10, HAL_MAX_DELAY);
		// GUI CODE END 2
	}
}

void DebugLog(const char* s)
{
//	return;
	HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), 100);
}
