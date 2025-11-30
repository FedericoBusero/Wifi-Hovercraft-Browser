#ifndef LOWPASS_FILTER_H
#define LOWPASS_FILTER_H

#include <Arduino.h>

/**
 *  Low pass filter class
 *  More information: https://docs.simplefoc.com/low_pass_filter
 */
class LowPassFilter
{
public:
    /**
     * @param Tf - Low pass filter time constant
     */
    LowPassFilter(float _Tf)
        : Tf(_Tf), y_prev(0.0f)
    {
        timestamp_prev = micros();
    }

    ~LowPassFilter() = default;

    float operator()(float x)
    {
        unsigned long timestamp = micros();
        float dt = (timestamp - timestamp_prev) * 1e-6f;

        if (dt < 0.0f)
            dt = 1e-3f;
        else if (dt > 0.3f)
        {
            y_prev = x;
            timestamp_prev = timestamp;
            return x;
        }

        float alpha = Tf / (Tf + dt);
        float y = alpha * y_prev + (1.0f - alpha) * x;
        y_prev = y;
        timestamp_prev = timestamp;
        return y;
    }

    float Tf; //!< Low pass filter time constant

    float getLastValue() { return y_prev; }

protected:
    unsigned long timestamp_prev; //!< Last execution timestamp
    float y_prev;                 //!< filtered value in previous execution step
};

#endif // LOWPASS_FILTER_H
