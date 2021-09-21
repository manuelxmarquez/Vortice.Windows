#include "Library.h"


// C++ XAudio2Test2.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <stdlib.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <xaudio2fx.h>
#include <xapofx.h>
#include <tchar.h>

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

IXAudio2* pXAudio2 = nullptr;
IXAudio2MasteringVoice* pMasterVoice = nullptr;
IXAudio2SourceVoice* pSourceVoice = nullptr;
XAUDIO2_BUFFER buffer = { 0 };
XAUDIO2FX_REVERB_PARAMETERS reverbParameters
{
reverbParameters.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY,
reverbParameters.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY,
reverbParameters.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY,
reverbParameters.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION,
reverbParameters.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION,
reverbParameters.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX,
reverbParameters.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX,
reverbParameters.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION,
reverbParameters.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION,
reverbParameters.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN,
reverbParameters.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF,
reverbParameters.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN,
reverbParameters.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF,
reverbParameters.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ,
reverbParameters.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN,
reverbParameters.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF,
reverbParameters.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN,
reverbParameters.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN,
reverbParameters.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME,
reverbParameters.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY,
reverbParameters.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE,
reverbParameters.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX
};


HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK)
    {
        DWORD dwRead;
        if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
            hr = HRESULT_FROM_WIN32(GetLastError());

        switch (dwChunkType)
        {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
                hr = HRESULT_FROM_WIN32(GetLastError());
            break;

        default:
            if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
                return HRESULT_FROM_WIN32(GetLastError());
        }

        dwOffset += sizeof(DWORD) * 2;

        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;

}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());
    DWORD dwRead;
    if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}


HRESULT PlayFile(LPCWSTR filename)
{
    std::cout << "XAudio2 Init ...\n";

    HRESULT hr;
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        return hr;

    IXAudio2* pXAudio2 = nullptr;
    if (FAILED(hr = XAudio2Create(&pXAudio2, 1, XAUDIO2_DEFAULT_PROCESSOR)))
        return hr;

    IXAudio2MasteringVoice* pMasterVoice = nullptr;
    if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice, 1, 48000)))
        return hr;

    std::cout << "XAudio2 Init Done\n";
    //getchar();
    std::cout << "Loading WAV Data ...\n";

    WAVEFORMATEXTENSIBLE wfx = { 0 };
    XAUDIO2_BUFFER buffer = { 0 };

#ifdef _XBOX
    //char* strFileName = "game:\\media\\MusicMono.wav";
#else
    LPCWSTR strFileName = filename;

#endif
    // Open the file
    HANDLE hFile = CreateFile(
        strFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
        return HRESULT_FROM_WIN32(GetLastError());

    if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
        return HRESULT_FROM_WIN32(GetLastError());

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    //check the file type, should be fourccWAVE or 'XWMA'
    FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    DWORD filetype;
    ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
    if (filetype != fourccWAVE)
        return S_FALSE;

    FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

    std::cout << "Loading WAV Data Done\n";
    //getchar();
    std::cout << "Filling XAudio2 Buffer & Setting Reverb ...\n";

    //fill out the audio data buffer with the contents of the fourccDATA chunk
    FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
    BYTE* pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
    buffer.pAudioData = pDataBuffer;  //buffer containing audio data
    buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

    IXAudio2SourceVoice* pSourceVoice;
    if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx, 1))) return hr;

    IUnknown* pXAPO;
    hr = XAudio2CreateReverb(&pXAPO, 0);

    XAUDIO2_EFFECT_DESCRIPTOR descriptor;
    descriptor.InitialState = true;
    descriptor.OutputChannels = 1;
    descriptor.pEffect = pXAPO;

    XAUDIO2_EFFECT_CHAIN chain;
    chain.EffectCount = 1;
    chain.pEffectDescriptors = &descriptor;

    pSourceVoice->SetEffectChain(&chain);

    // Step5???

    XAUDIO2FX_REVERB_PARAMETERS reverbParameters;
    reverbParameters.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY;
    reverbParameters.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY;
    reverbParameters.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
    reverbParameters.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
    reverbParameters.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
    reverbParameters.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
    reverbParameters.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
    reverbParameters.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
    reverbParameters.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
    reverbParameters.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
    reverbParameters.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
    reverbParameters.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
    reverbParameters.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
    reverbParameters.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ;
    reverbParameters.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN;
    reverbParameters.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF;
    reverbParameters.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN;
    reverbParameters.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN;
    reverbParameters.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME;
    reverbParameters.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY;
    reverbParameters.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE;
    reverbParameters.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX;

    hr = pSourceVoice->SetEffectParameters(0, &reverbParameters, sizeof(reverbParameters));
    pSourceVoice->EnableEffect(0);

    std::cout << "Filling XAudio2 Buffer & Setting Reverb Done\n";
    //getchar();
    std::cout << "Starting XAudio2 - Playing WAV ...\n";

    if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
        return hr;

    if (FAILED(hr = pSourceVoice->Start(0)))
        return hr;

    std::cout << "Starting XAudio2 - Playing WAV Done\n";
    //getchar();
}

// START HERE
void InitXAudio2(int Channels, int SampleRate)
{
    std::cout << "XAudio2 Init ...\n";

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    pXAudio2->CreateMasteringVoice(&pMasterVoice, Channels, SampleRate);

    std::cout << "XAudio2 Init Done\n";
    //getchar();
}

void LoadAudio(const char* filename)
{
    std::cout << "Loading WAV Data ...\n";

    WAVEFORMATEXTENSIBLE wfx = { 0 };
    //XAUDIO2_BUFFER buffer = { 0 };

    const char* strFileName = filename;

    // Open the file
    HANDLE hFile = CreateFileA(
        strFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    DWORD dwChunkSize;
    DWORD dwChunkPosition;
    //check the file type, should be fourccWAVE or 'XWMA'
    FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
    DWORD filetype;
    ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);

    FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
    ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

    std::cout << "Loading WAV Data Done\n";
    //getchar();
    std::cout << "Filling XAudio2 Buffer & Setting Reverb ...\n";

    //fill out the audio data buffer with the contents of the fourccDATA chunk
    FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
    BYTE* pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

    buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
    buffer.pAudioData = pDataBuffer;  //buffer containing audio data
    buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

    pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx);

    pSourceVoice->SubmitSourceBuffer(&buffer);

    IUnknown* pXAPO;
    XAudio2CreateReverb(&pXAPO, 0);

    XAUDIO2_EFFECT_DESCRIPTOR descriptor;
    descriptor.InitialState = true;
    descriptor.OutputChannels = 1;
    descriptor.pEffect = pXAPO;

    XAUDIO2_EFFECT_CHAIN chain;
    chain.EffectCount = 1;
    chain.pEffectDescriptors = &descriptor;

    pSourceVoice->SetEffectChain(&chain);

    // Step5???    
}

void SetReverbParameters()
{
    XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset = XAUDIO2FX_I3DL2_PRESET_CAVE;

    ReverbConvertI3DL2ToNative(&preset, &reverbParameters, 0);

    pSourceVoice->SetEffectParameters(0, &reverbParameters, sizeof(reverbParameters));
}

void EnableReverb(bool value)
{
    std::cout << "Enable Reverb ...\n";
    if (value == true)
        pSourceVoice->EnableEffect(0);
    else
        pSourceVoice->DisableEffect(0);
}

bool PlayFile()
{
    std::cout << "Starting XAudio2 - Playing WAV ...\n";
    pSourceVoice->Start(0);
    getchar();
    return true;
}

bool StopFile()
{
    pSourceVoice->Stop(0);
    //getchar();
    return false;
}

void CleanUp()
{
    pSourceVoice->DestroyVoice();
    pMasterVoice->DestroyVoice();
    pXAudio2->Release();
    CoUninitialize();
    std::cout << "Goodbye :)";
    //getchar();
}

int main()
{
    InitXAudio2(1, 48000);
    LoadAudio(".\\Test.wav");
    SetReverbParameters();
    EnableReverb(true);
    PlayFile();
    StopFile();
    CleanUp();
}



extern int APIENTRY CreateEffect(void* ptr)
{
    IXAudio2SourceVoice* pSourceVoice = (IXAudio2SourceVoice*)ptr;

    //IUnknown* pXAPO;
    //XAudio2CreateReverb(&pXAPO, 0);

    //XAUDIO2_EFFECT_DESCRIPTOR descriptor;
    //descriptor.InitialState = true;
    //descriptor.OutputChannels = 1;
    //descriptor.pEffect = pXAPO;

    //XAUDIO2_EFFECT_CHAIN chain;
    //chain.EffectCount = 1;
    //chain.pEffectDescriptors = &descriptor;

    //if (!SUCCEEDED(pSourceVoice->SetEffectChain(&chain)))
    //    return -1;


    XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset = XAUDIO2FX_I3DL2_PRESET_CAVE;

    ReverbConvertI3DL2ToNative(&preset, &reverbParameters, 0);

    if (!SUCCEEDED(pSourceVoice->SetEffectParameters(0, &reverbParameters, sizeof(reverbParameters))))
        return -1;


    //if (!SUCCEEDED(pSourceVoice->EnableEffect(0)))
    //    return -1;

    return 0;
}

extern int APIENTRY GetSize()
{
    return sizeof(XAUDIO2FX_REVERB_PARAMETERS);
}

extern XAUDIO2FX_REVERB_PARAMETERS APIENTRY GetDataStruct()
{
    XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset = XAUDIO2FX_I3DL2_PRESET_CAVE;

    XAUDIO2FX_REVERB_PARAMETERS parameters = {};
   /* {
        parameters.ReflectionsDelay = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY,
        parameters.ReverbDelay = XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY,
        parameters.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY,
        parameters.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION,
        parameters.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION,
        parameters.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX,
        parameters.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX,
        parameters.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION,
        parameters.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION,
        parameters.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN,
        parameters.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF,
        parameters.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN,
        parameters.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF,
        parameters.RoomFilterFreq = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ,
        parameters.RoomFilterMain = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN,
        parameters.RoomFilterHF = XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF,
        parameters.ReflectionsGain = XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN,
        parameters.ReverbGain = XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN,
        parameters.DecayTime = XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME,
        parameters.Density = XAUDIO2FX_REVERB_DEFAULT_DENSITY,
        parameters.RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE,
        parameters.WetDryMix = XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX
    };*/

    ReverbConvertI3DL2ToNative(&preset, &reverbParameters, 0);

    return reverbParameters;
}
