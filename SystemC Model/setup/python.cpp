#include <setup/python.hpp>
#include <iostream>

Python::Python()
    : pymodule(nullptr)
{
    // Check if the interpreter got already started (should not be)
    if(Py_IsInitialized())
    {
        const char *errormsg = "Embedded Python already initialized!";
        this->PrintPythonError(errormsg);
        throw std::logic_error(errormsg);
    }

    // Start Embedded Python Interpreter
    setenv("PYTHONPATH", ".:./setup", 1/*overwrite*/);
    Py_Initialize();
    return;
}



Python::~Python()
{
    this->ForceShutdown();
    return;
}



void Python::PrintPythonError(const char *msg)
{
    if(msg != nullptr)
    {
        std::cerr << "\e[1;31mEmbedded Python Interface Error:\e[0m "
                  << msg
                  << "\e[0m\n";
        std::cerr.flush();
    }

    if(PyErr_Occurred())
        PyErr_Print();

    return;
}



void Python::LoadModule(const char *modulename)
{
    const std::lock_guard<std::mutex> lock(this->mutex);

    // Unload module if the was one loaded before
    if(this->pymodule != nullptr)
    {
        Py_DECREF(this->pymodule);
        this->pymodule = nullptr;
    }

    // Do not load a new one, if no name is given
    if(modulename == nullptr)
        return;

    // Load new module
    PyObject *pymodulename;
    pymodulename = PyUnicode_DecodeFSDefault(modulename);
    if(pymodulename == nullptr)
    {
        const char *errormsg = "Encoding Python module name failed!";
        this->PrintPythonError(errormsg);
        throw std::runtime_error(errormsg);
    }

    this->pymodule = PyImport_Import(pymodulename);
    Py_DECREF(pymodulename);
    if(this->pymodule == nullptr)
    {
        const char *errormsg = "Loading Python module failed!";
        this->PrintPythonError(errormsg);
        throw std::runtime_error(errormsg);
    }

    return;
}



PyObject* Python::Call(const char *functionname, PyObject *args)
{
    const std::lock_guard<std::mutex> lock(this->mutex);

    if(functionname == nullptr)
    {
        const char *errormsg = "No function name given to call!";
        this->PrintPythonError(errormsg);
        throw std::logic_error(errormsg);
    }

    if(this->pymodule == nullptr)
    {
        const char *errormsg = "Call of object in unloaded Python module!";
        this->PrintPythonError(errormsg);
        throw std::runtime_error(errormsg);
    }

    // Loading Function
    PyObject *pyfunction;
    pyfunction = PyObject_GetAttrString(this->pymodule, functionname);
    if(pyfunction == nullptr)
    {
        const char *errormsg = "Loading Python object failed!";
        this->PrintPythonError(errormsg);
        Py_DECREF(pyfunction);
        throw std::runtime_error(errormsg);
    }
    if(PyCallable_Check(pyfunction) == 0)
    {
        const char *errormsg = "Python object not callable!";
        this->PrintPythonError(errormsg);
        Py_DECREF(pyfunction);
        throw std::runtime_error(errormsg);
    }

    // Call Function
    PyObject *pyretval;
    pyretval = PyObject_CallObject(pyfunction, args);
    if(pyretval == nullptr)
    {
        const char *errormsg = "Python function failed!";
        this->PrintPythonError(errormsg);
        Py_DECREF(pyfunction);
        throw std::runtime_error(errormsg);
    }

    // Free all memory
    Py_DECREF(pyfunction);
    return pyretval;
}

void Python::ForceShutdown()
{
    const std::lock_guard<std::mutex> lock(this->mutex);

    if(not Py_IsInitialized())
        return;

    if(this->pymodule != nullptr)
        Py_DECREF(this->pymodule);

    // Shutdown Python
    if(Py_FinalizeEx() < 0)
        this->PrintPythonError("Finalizing failed!");
}

