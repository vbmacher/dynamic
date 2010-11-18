/* 
 * File:   clprogram.cpp
 * Author: vbmacher
 * 
 * Created on Sunday, 2010, october 17, 8:09
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

//#define __CL_ENABLE_EXCEPTIONS

#include "clprogram.hpp"

using namespace std;

CLprogram::CLprogram() {
    error = CL_SUCCESS;
}

CLprogram::~CLprogram() {
}

bool CLprogram::initCL() {
    // Platform info
    cl::vector<cl::Platform> platforms;

    error = cl::Platform::get(&platforms);
    if (error != CL_SUCCESS) {
        cerr << "Platform::get() failed (" << error << ")" << endl;
        return false;
    }
    
    if (platforms.size() == 0) {
        cerr << "No compatible platforms found." << endl;
        return false;
    }

    /*
     * If we could find our platform, use it. Otherwise pass a NULL and get whatever the
     * implementation thinks we should be using.
     */

    cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, 
        (cl_context_properties)(platforms[0])(), 0};

    this->context = cl::Context(CL_DEVICE_TYPE_CPU, cps, NULL, NULL, &error);
    if (error != CL_SUCCESS) {
        cerr << "Context::Context() failed (" << error << ")\n";
        return false;
    }

    devices = context.getInfo<CL_CONTEXT_DEVICES > ();
    if (error != CL_SUCCESS) {
        cerr << "Context::getInfo() failed (" << error << ")\n";
        return false;
    }
    if (devices.size() == 0) {
        cerr << "No device available\n";
        return false;
    }
    queue = cl::CommandQueue(context, devices[0], 0, &error);
    if (error != CL_SUCCESS) {
        cerr << "CommandQueue::CommandQueue() failed (" << error << ")\n";
    }
    return true;
}

bool CLprogram::loadProgram(const char* source) {
    if (error != CL_SUCCESS)
        return false;
    cl::Program::Sources sources(1, make_pair(source, strlen(source)));

    program = cl::Program(context, sources, &error);
    if (error != CL_SUCCESS) {
        cerr << "Program::Program() failed (" << error << ")\n";
        return false;
    }

    error = program.build(devices);
    if (error != CL_SUCCESS) {

        if (error == CL_BUILD_PROGRAM_FAILURE) {
            cl::string str = program.getBuildInfo<CL_PROGRAM_BUILD_LOG > (devices[0]);

            cout << " \n\t\t\tBUILD LOG\n";
            cout << " ************************************************\n";
            cout << str.c_str() << endl;
            cout << " ************************************************\n";
        }

        cerr << "Program::build() failed (" << error << ")\n";
        return false;
    }
    return true;
}

cl::Kernel CLprogram::getKernel(const char* name) {
    cl::Kernel kernel(program, name, &error);
    if (error != CL_SUCCESS) {
        cerr << "Kernel::Kernel() failed (" << error << ")\n";
        throw new exception;
    }
    return kernel;
}

bool CLprogram::runKernel(cl::Kernel kernel) {
    if (error != CL_SUCCESS)
        return false;
    error = queue.enqueueNDRangeKernel(kernel, cl::NullRange,
            cl::NDRange(1), cl::NullRange);

    if (error != CL_SUCCESS) {
        cerr << "CommandQueue::enqueueNDRangeKernel()" \
            " failed (" << error << ")\n";
        return false;
    }

    error = queue.finish();
    if (error != CL_SUCCESS) {
        cerr << "Event::wait() failed (" << error << ")\n";
        return false;
    }
    return true;
}
