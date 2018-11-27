#include <stdio.h>
#include <string.h>


#define NumberOf(a) (sizeof (a) / sizeof *(a))


/*  Convolve Signal with Filter.

    Signal must contain OutputLength + FilterLength - 1 elements.  Conversely,
    if there are N elements in Signal, OutputLength may be at most
    N+1-FilterLength.
*/
static void convolve(
    float *Signal,
    float *Filter, size_t FilterLength,
    float *Output, size_t OutputLength)
{
    for (size_t i = 0; i < OutputLength; ++i)
    {
        double sum = 0;
        for (size_t j = 0; j < FilterLength; ++j)
            sum += Signal[i+j] * Filter[FilterLength - 1 - j];
        Output[i] = sum;
    }
}


int main(void)
{
    //  Define a length for buffers that is long enough for this demonstration.
    #define LongEnough  128


    //  Define some sample filters.
    float Filter0[] = { 1, 2, -1 };
    float Filter1[] = { 1, 5, 7, 5, 1 };

    size_t Filter0Length = NumberOf(Filter0);
    size_t Filter1Length = NumberOf(Filter1);


    //  Define a unit impulse positioned so it captures all of the filters.
    size_t UnitImpulsePosition = Filter0Length - 1 + Filter1Length - 1;
    float UnitImpulse[LongEnough];
    memset(UnitImpulse, 0, sizeof UnitImpulse);
    UnitImpulse[UnitImpulsePosition] = 1;


    //  Calculate a filter that is Filter0 and Filter1 combined.
    float CombinedFilter[LongEnough];

    //  Set N to number of inputs that must be used.
    size_t N = UnitImpulsePosition + 1 + Filter0Length - 1 + Filter1Length - 1;

    //  Subtract to find number of outputs of first convolution, then convolve.
    N -= Filter0Length - 1;
    convolve(UnitImpulse,    Filter0, Filter0Length, CombinedFilter, N);

    //  Subtract to find number of outputs of second convolution, then convolve.
    N -= Filter1Length - 1;
    convolve(CombinedFilter, Filter1, Filter1Length, CombinedFilter, N);

    //  Remember size of resulting filter.
    size_t CombinedFilterLength = N;

    //  Display filter.
    for (size_t i = 0; i < CombinedFilterLength; ++i)
        printf("CombinedFilter[%zu] = %g.\n", i, CombinedFilter[i]);


    //  Define two identical signals.
    float Buffer0[LongEnough];
    float Buffer1[LongEnough];
    for (size_t i = 0; i < LongEnough; ++i)
    {
        Buffer0[i] = i;
        Buffer1[i] = i;
    }


    //  Convolve Buffer0 by using the two filters separately.

    //  Start with buffer length.
    N = LongEnough;

    //  Subtract to find number of outputs of first convolution, then convolve.
    N -= Filter0Length - 1;
    convolve(Buffer0, Filter0, Filter0Length, Buffer0, N);

    //  Subtract to find number of outputs of second convolution, then convolve.
    N -= Filter1Length - 1;
    convolve(Buffer0, Filter1, Filter1Length, Buffer0, N);

    //  Remember the length of the result.
    size_t ResultLength = N;


    //  Convolve Buffer1 with the combined filter.
    convolve(Buffer1, CombinedFilter, CombinedFilterLength, Buffer1, ResultLength);


    //  Show the contents of Buffer0 and Buffer1, and their differences.
    for (size_t i = 0; i < ResultLength; ++i)
    {
        printf("Buffer0[%zu] = %g.  Buffer1[%zu] = %g.  Difference = %g.\n",
            i, Buffer0[i], i, Buffer1[i], Buffer0[i] - Buffer1[i]);
    }

    return 0;
}
