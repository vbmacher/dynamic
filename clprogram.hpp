/* 
 * File:   clprogram.hpp
 * Author: vbmacher
 *
 * Created on Sunday, 2010, october 17, 8:09
 */

#ifndef CLPROGRAM_HPP
#define	CLPROGRAM_HPP

#define __NO_STD_VECTOR
#define __NO_STD_STRING

#include <CL/cl.hpp>

class CLprogram {
private:
    cl_int error;
    cl::Context context;
    cl::vector<cl::Device> devices;
    cl::Program program;
    cl::CommandQueue queue;
public:
    CLprogram();
    virtual ~CLprogram();
    bool initCL();
    cl_int getError() { return error; }
    bool loadProgram(const char *source);
    cl::Kernel getKernel(const char *name);
    bool runKernel(cl::Kernel kernel, cl::Event *event = NULL);
    bool finishAll();
    const cl::Context &getContext() { return context; }
    const cl::CommandQueue &getCommandQueue() { return queue; }
};

#endif	/* CLPROGRAM_HPP */

