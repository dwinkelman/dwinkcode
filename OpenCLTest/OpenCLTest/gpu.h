#ifndef GPU_H
#define GPU_H

#include <utility>
#include <CL/cl.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <iterator>

inline void check_err(cl_int err, const std::string name) {
	if (err != CL_SUCCESS) {
		std::cerr << "Error: " << name << " (" << err << ")\n";
		// exit(EXIT_FAILURE);
	}
}

inline void __stdcall device_err_callback(const char * err_info, const void * private_info, size_t cb, void * user_data) {
	std::cout << "Error: Device: " << err_info << '\n';
}

class Context {
protected:
	cl_context context;
	cl_device_id devices[16];
	unsigned int n_devices;

public:
	Context() {
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
			const struct device_info_params {
				cl_device_info param;
				const char * name;
			} params[8]{
				{ CL_DEVICE_NAME, "Name" },
				{ CL_DEVICE_ADDRESS_BITS, "Address" },
				{ CL_DEVICE_GLOBAL_MEM_SIZE, "Global Memory Size" },
				{ CL_DEVICE_LOCAL_MEM_SIZE, "Local Memory Size" },
				{ CL_DEVICE_MAX_COMPUTE_UNITS, "Compute Units" },
				{ CL_DEVICE_MAX_WORK_GROUP_SIZE, "Work Group Size" },
				{ CL_DEVICE_MAX_WORK_ITEM_SIZES, "Work Item Sizes" },
				{ CL_DEVICE_PROFILING_TIMER_RESOLUTION, "Timer Resolution (ns)" }
			};

			// Get a list of GPUs
			cl_device_id gpus[16];
			unsigned int n_gpus;
			clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 16, gpus, &n_gpus);

			// Display information for each device
			for (unsigned int j = 0; j < n_gpus; j++) {
				std::cout << "\tGPU " << j << '\n';
				for (int k = 0; k < 8; k++) {
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
				for (int k = 0; k < 8; k++) {
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

	inline cl_context get_context() {
		return context;
	}
	inline unsigned int get_n_devices() {
		return n_devices;
	}
	inline cl_device_id * get_devices() {
		return devices;
	}
};

class Kernel {
protected:
	Context * context;
	cl_kernel kernel;

public:
	Kernel() {
		context = NULL;
		kernel = NULL;
	}

	Kernel(Context * context, const std::string path, const std::string kernel_name) {
		this->context = context;

		// Error code output
		cl_int err;

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

		const char * program_src_cs = program_src.c_str();

		// Create the program
		cl_program program = clCreateProgramWithSource(context->get_context(), 1, &program_src_cs, NULL, &err);
		check_err(err, "clCreateProgramWithSource " + path);

		// Build the program
		err = clBuildProgram(program, 1, context->get_devices(), NULL, NULL, NULL);
		check_err(err, "clBuildProgram " + path);

		// Print the build logs
		for (unsigned int i = 0; i < context->get_n_devices(); i++) {
			char buffer[1024];
			err = clGetProgramBuildInfo(program, context->get_devices()[i], CL_PROGRAM_BUILD_LOG, 1024, buffer, NULL);
			check_err(err, "clGetProgramBuildInfo");
			std::cout << "Build Log for Device " << i << ":\n" << buffer << "\n";
		}

		// Create the kernel
		kernel = clCreateKernel(program, kernel_name.c_str(), &err);
		check_err(err, "clCreateKernel " + path + ' ' + kernel_name);
	}
};

class MatrixMultiplicationKernel : public Kernel {
public:
	MatrixMultiplicationKernel() {

	}

	MatrixMultiplicationKernel(Context * context) : Kernel(context, "matrix.cl", "matrix_multiply") {

	}

	void multiply(cl_command_queue cmd_q,
		float * a, float * b, float * c,
		unsigned int left_R, unsigned int left_C, unsigned int right_C);
};

#endif