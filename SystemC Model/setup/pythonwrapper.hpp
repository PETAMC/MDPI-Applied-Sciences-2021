#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <setup/python.hpp>

// Singleton pattern: https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class PythonWrapper
{
    public:
        static PythonWrapper& GetInstance()
        {
            static PythonWrapper instance;
            return instance;
        }
        PythonWrapper();

        PythonWrapper(PythonWrapper const&)  = delete;
        void operator=(PythonWrapper const&) = delete;
        
        ~PythonWrapper();
        void ForceShutdown();

        // Python Functions
        int GaussianKDE(std::vector<double> *samples, std::string &datapath);

    private:
        void PythonThread();

        std::mutex access; // For function wrapper to request exclusive access to the thread

        // Change the following parameters only inside the thread,
        // or when owning the access Mutex.
        struct
        {
            std::atomic<bool> run;      // Run the thread if true
            std::atomic<bool> call;     // Call function if true. Gets set to false by the thread after being back
            const char *functionname;   // Set by function

            // GaussianKDE parameters
            std::string *datapath;            // Input for the function
            std::vector<double> *returnvalue; // The underlying vector gets updated in the thread 
                                              // with the returned data.
                                              // In case of an error, the pointer gets replaced by nullptr
        } threadstate;
        std::thread pythonthread;

};

#endif

