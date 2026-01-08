#define MAIN_TEST_TIMING 0

#if MAIN_TEST_TIMING

#include <algorithm>
#include <array>
#include <cmath>

constexpr unsigned long PERIOD = 1000;

unsigned long previousMillis;
unsigned long loopCounter = 0;
unsigned long counterIndex = 0;

std::array<unsigned long, PERIOD> loopCounters{};

template <size_t VSize>
double GetDeviation(double average, const std::array<unsigned long, VSize>& values, double& outMaxError)
{
    double sumOfSquaredErrors = 0.0;
    double maxError = 0.0;
    for (const unsigned long value : values)
    {
        const double error = std::fabs(average - static_cast<double>(value));
        if (maxError < error)
            maxError = error;
        sumOfSquaredErrors += error * error;
    }

    outMaxError = maxError;
    return sumOfSquaredErrors / static_cast<double>(values.size() - 1);
}

void setup()
{
    Serial.begin(9600);
    previousMillis = millis();
}

void loop()
{
    unsigned long currentMillis = millis();

    if (previousMillis != currentMillis)
    {
        counterIndex = (counterIndex + 1) % PERIOD;
    }

    if (previousMillis + PERIOD < currentMillis)
    {
        const double averageLoopFrequency = static_cast<double>(loopCounter) / PERIOD;
        double maxError = 0.0;
        const double deviation = GetDeviation(averageLoopFrequency, loopCounters, maxError);
        Serial.printf("Looped %u times in %ums => %f loop/ms, dev: %e, max: %e\n",
            loopCounter, PERIOD, averageLoopFrequency, deviation, maxError);

        std::fill(loopCounters.begin(), loopCounters.end(), 0);
        loopCounter = 0;
        previousMillis = millis();
    }
    else
    {
        loopCounter++;
        loopCounters[counterIndex]++;
    }
}

#endif
