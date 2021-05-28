#include <setup/pythonwrapper.hpp>
#include <cstdio>
#include <chrono>
#include <iostream>


PythonWrapper::PythonWrapper()
{
    this->threadstate.datapath    = nullptr;
    this->threadstate.returnvalue = nullptr;
    this->threadstate.call        = false;
    this->threadstate.run         = true;
    std::cerr << "\e[1;34mStarting Python thread\e[0m\n";
    this->pythonthread = std::thread(&PythonWrapper::PythonThread, this);
}

PythonWrapper::~PythonWrapper()
{
    this->ForceShutdown();
}


void PythonWrapper::ForceShutdown()
{
    if(not this->threadstate.run)
        return;

    std::cerr << "\e[1;34mStopping Python thread\e[0m\n";

    this->threadstate.run = false;
    this->pythonthread.join();

    Python &python = Python::GetInstance();
    python.ForceShutdown();
}


int PythonWrapper::GaussianKDE(std::vector<double> *samples, std::string &datapath)
{
    // Request access
    std::lock_guard<std::mutex> guard(this->access);

    // Prepare call
    this->threadstate.datapath     = &datapath;
    this->threadstate.returnvalue  = samples;
    this->threadstate.functionname = "GaussianKDE";

    // Process Call
    this->threadstate.call = true;
    while(this->threadstate.call);  // Wait until finish

    // Check for success
    if(this->threadstate.returnvalue == nullptr)
        return -1;  // Call was not successful - error message @ stderr

    return 0;
}


void PythonWrapper::PythonThread()
{
    // Start Python Setup
    Python &python = Python::GetInstance();
    python.LoadModule("kde");

    // Entering loop that waits for function call requests
    while(this->threadstate.run)
    {
        if(not this->threadstate.call)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Prepare arguments
        PyObject *pyfilepath;
        PyObject *arguments;

        pyfilepath = PyUnicode_DecodeFSDefault(this->threadstate.datapath->c_str());
        if(pyfilepath == nullptr)
        {
            std::cerr << "\e[1;31mCreating Python Object for const char* failed!\e[0m\n";
            this->threadstate.returnvalue = nullptr;
            this->threadstate.call        = false;
            continue;
        }

        // Create argument tuple
        arguments = PyTuple_New(1);
        PyTuple_SetItem(arguments, 0, pyfilepath);

        // Call
        PyObject *returnvalue;
        returnvalue = python.Call(this->threadstate.functionname, arguments);

        // Process returned data
        if(PyList_Check(returnvalue) == 0)
        {
            std::cerr << "\e[1;31mPython function did not return an expected list!\e[0m\n";
            Py_DECREF(arguments);
            arguments = nullptr;
            this->threadstate.returnvalue = nullptr;
            this->threadstate.call        = false;
            continue;
        }

        Py_ssize_t listsize;
        listsize = PyList_Size(returnvalue);
        this->threadstate.returnvalue->clear();
        this->threadstate.returnvalue->reserve(static_cast<size_t>(listsize));

        for(Py_ssize_t i = 0; i < listsize; i++)
        {
            PyObject *pyvalue = PyList_GetItem(returnvalue, i);
            double value;
            value = PyFloat_AsDouble(pyvalue);
            this->threadstate.returnvalue->push_back(value);
        }
        
        // Clean everything
        Py_DECREF(arguments);
        arguments = nullptr;
        Py_DECREF(returnvalue);
        returnvalue = nullptr;

        // Call done
        this->threadstate.call = false;
    }

    // Leaving thread
}

