#pragma once

#include <windows.h>
#include <xaudio2fx.h>


#define EXPORT __declspec(dllexport)

extern "C"
{
    EXPORT extern int APIENTRY CreateEffect(void* ptr);

    EXPORT extern XAUDIO2FX_REVERB_PARAMETERS APIENTRY GetDataStruct();
    EXPORT extern int APIENTRY GetSize();
}
