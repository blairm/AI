#ifndef PLATFORM_H
#define PLATFORM_H

namespace Platform
{
    extern void Log( char* string, ... );
    #define LOG( string, ... ) Platform::Log( string, ##__VA_ARGS__ )


    extern f64 GetTime();


    extern s32 GetProcessorCount();

    typedef void WorkCallback( s32 threadIndex, void* data );
    extern void AddWorkQueueEntry( WorkCallback* callback, void* data );
    extern void FinishWork();
}


#endif