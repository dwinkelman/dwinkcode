#include "gpu.h"

Context::Context() {
	// Error code output
	cl_int err;

	// Generate a list of platforms
	cl_platform_id platforms[16];
	unsigned int n_platforms;
	clGetPlatformIDs(16, platforms, &n_platforms);
	std::cout << "Found " << n_platforms << " platforms\n";

	// Print information for each platform
	for (unsigned int i = 0; i < n_platforms; i++) {
		// Get some statistic for the platform
		char buffer[1024];
		clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 1024, buffer, NULL);
		std::cout << "Platform " << i << ": " << buffer << "\n";

		// Print information for each GPU
		int _INT = 0;
		int _STR = 0;
		const struct device_info_params {
			cl_device_info param;
			const char * name;
			int output_type;
		} params[]{
			{ CL_DEVICE_NAME, "Name", _STR },
			{ CL_DEVICE_VERSION, "Version", _STR },
			{ CL_DEVICE_DOUBLE_FP_CONFIG, "Supports Doubles", _INT },
			{ CL_DEVICE_ADDRESS_BITS, "Address", _INT },
			{ CL_DEVICE_GLOBAL_MEM_SIZE, "Global Memory Size", _INT },
			{ CL_DEVICE_LOCAL_MEM_SIZE, "Local Memory Size", _INT },
			{ CL_DEVICE_MAX_COMPUTE_UNITS, "Compute Units", _INT },
			{ CL_DEVICE_MAX_WORK_GROUP_SIZE, "Work Group Size", _INT },
			{ CL_DEVICE_MAX_WORK_ITEM_SIZES, "Work Item Sizes", _INT },
			{ CL_DEVICE_PROFILING_TIMER_RESOLUTION, "Timer Resolution (ns)", _INT }
		};

		// Get a list of GPUs
		cl_device_id gpus[16];
		unsigned int n_gpus;
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 16, gpus, &n_gpus);

		// Display information for each device
		for (unsigned int j = 0; j < n_gpus; j++) {
			std::cout << "\tGPU " << j << '\n';
			for (int k = 0; k < 10; k++) {
				uint64_t output;
				clGetDeviceInfo(gpus[j], params[k].param, sizeof(output), &output, NULL);
				std::cout << "\t\t" << params[k].name << ": " << output << '\n';
			}
		}

		// Get a list of CPUs
		cl_device_id cpus[16];
		unsigned int n_cpus;
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 16, cpus, &n_cpus);

		// Display information for each device
		for (unsigned int j = 0; j < n_cpus; j++) {
			std::cout << "\tCPU " << j << '\n';
			for (int k = 0; k < 10; k++) {
				uint64_t output;
				clGetDeviceInfo(cpus[j], params[k].param, sizeof(output), &output, NULL);
				std::cout << "\t\t" << params[k].name << ": " << output << '\n';
			}
		}
	}

	// Get all the GPUs for the first platform and create into a context
	std::cout << "\nSelecting Platform 0\n\n";
	err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 16, devices, &n_devices);
	check_err(err, "clGetDeviceIDs");
	// context = clCreateContext(NULL, n_devices, devices, device_err_callback, NULL, &err);
	context = clCreateContext(NULL, n_devices, devices, NULL, NULL, &err);
	check_err(err, "clCreateContext");
}

static std::string read_file(const std::string path) {
	// Read file
	std::string program_src;
	std::ifstream ifs(path);
	if (ifs) {
		std::stringstream ss;
		ss << ifs.rdbuf();
		ifs.close();
		program_src = ss.str();
	}
	else {
		check_err(1, "Failed to load file " + path);
	}
	return program_src;
}

Kernel::Kernel(Context * context, const char * source, const std::string kernel_name, const char * compiler_options = "") {
	this->context = context;

	// Error code output
	cl_int err;

	// Create the program
	cl_program program = clCreateProgramWithSource(context->get_context(), 1, &source, NULL, &err);
	check_err(err, "clCreateProgramWithSource");

	// Build the program
	err = clBuildProgram(program, context->get_n_devices(), context->get_devices(), NULL, NULL, NULL);
	check_err(err, "clBuildProgram");

	// Print the build logs
	for (unsigned int i = 0; i < context->get_n_devices(); i++) {
		char buffer[1024] = "Null\x0";
		err = clGetProgramBuildInfo(program, context->get_devices()[i], CL_PROGRAM_BUILD_LOG, 1024, buffer, NULL);
		check_err(err, "clGetProgramBuildInfo");
		std::cout << "Build Log for Device " << i << ":\n" << buffer << "\n";
	}

	// Create the kernel
	kernel = clCreateKernel(program, kernel_name.c_str(), &err);
	check_err(err, "clCreateKernel " + kernel_name);
}

Kernel::Kernel(Context * context, const std::string path, const std::string kernel_name, const char * compiler_options = "")
	: Kernel(context, read_file(path).c_str(), kernel_name, compiler_options) { }

// See "matrix.cl"
const char * matrix_kernel =
"__kernel void matrix_multiply("
"	__global float * A,"
"	__global float * B,"
"	__global float * C,"
"	int a_cols, int b_cols"
") {"
"\n#define BLOCK_SIZE 16\n"
""
"	int block_x = get_group_id(0);"
"	int block_y = get_group_id(1);"
""
"	int thread_x = get_local_id(0);"
"	int thread_y = get_local_id(1);"
""
"	int global_x = get_global_id(0);"
"	int global_y = get_global_id(1);"
""
"	int a_begin = a_cols * BLOCK_SIZE * block_y;"
"	int a_end = a_begin + a_cols - 1;"
"	int a_step = BLOCK_SIZE;"
""
"	int b_begin = BLOCK_SIZE * block_x;"
"	int b_step = BLOCK_SIZE * b_cols;"
""
"	float c_sub = 0;"
""
"	for (int a = a_begin, b = b_begin;"
"		a < a_end;"
"		a += a_step, b += b_step)"
"	{"
"		__local float a_sub[BLOCK_SIZE][BLOCK_SIZE];"
"		__local float b_sub[BLOCK_SIZE][BLOCK_SIZE];"
""
"		a_sub[thread_y][thread_x] = A[a + a_cols * thread_y + thread_x];"
"		b_sub[thread_y][thread_x] = B[b + b_cols * thread_y + thread_x];"
""
"		barrier(CLK_LOCAL_MEM_FENCE);"
""
"\n#pragma unroll\n"
"		for (int k = 0; k < BLOCK_SIZE; ++k)"
"			c_sub += a_sub[thread_y][k] * b_sub[k][thread_x];"
""
"		barrier(CLK_LOCAL_MEM_FENCE);"
"	}"
""
"	C[global_y * b_cols + global_x] = c_sub;"
"}";