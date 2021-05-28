#ifndef PYTHON_HPP
#define PYTHON_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <exception>
#include <stdexcept>
#include <mutex>

// Singleton pattern: https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class Python
{
    public:
        static Python& GetInstance()
        {
            static Python instance;
            return instance;
        }
        ~Python();

        Python(Python const&)         = delete;
        void operator=(Python const&) = delete;

        void LoadModule(const char *modulename);
        // If there is a module loaded, it gets unloaded before
        // LoadModule(nullptr) unloads the module only

        PyObject* Call(const char *functionname, PyObject *args = nullptr);
        // function name should not be nullptr

        void ForceShutdown(); // ! This method shuts down the python interpreter.
                                // After calling this function, all method called
                                // to this class are invalid/undefined!

    private:
        Python();

        void PrintPythonError(const char *msg = nullptr);

        PyObject *pymodule;
        std::mutex mutex;
};

#endif

