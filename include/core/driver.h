#ifndef DRIVER_H
#define DRIVER_H


    class Driver
    {
        public:
            Driver();
            ~Driver();
            const char* driverName; // 25 chrs
            virtual void Activate();
            virtual int Reset();
            virtual void Deactivate();
    };

    class DriverManager
    {
        private:
            Driver* drivers[255];
            int numDrivers;
        public:
            DriverManager();
            void AddDriver(Driver*);
            void ActivateAll();
    };


#endif // DRIVER_H