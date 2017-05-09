#include <windows.h>

int usleep (int useconds)
{
    int mseconds = useconds / 1000;
    useconds = useconds % 1000;
    if ( mseconds> 0) {
        /* At least one second. Millisecond resolution is sufficient. */
        Sleep ( mseconds );
    }

    /* Use Sleep for the largest part, and busy-loop for the rest. */
    if (useconds>0) {

        static double usec_ticks_per_sec;
        if (usec_ticks_per_sec  == 0)
        {
            LARGE_INTEGER freq;
            if (!QueryPerformanceFrequency (&freq))
            {
                /* Cannot use QueryPerformanceCounter. */
                return 0;
            }
            usec_ticks_per_sec = (double) freq.QuadPart / 1000000.0;
        }

        long long expected_counter_difference = useconds * usec_ticks_per_sec;
        LARGE_INTEGER before;
        QueryPerformanceCounter (&before);
        long long expected_counter = before.QuadPart + expected_counter_difference;

        for (;;)
        {
            LARGE_INTEGER after;
            QueryPerformanceCounter (&after);
            if (after.QuadPart >= expected_counter)
                break;
        }
    }

    return 0;
}

