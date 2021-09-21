using System;
using System.Runtime.InteropServices;
using Vortice.Multimedia;
using Vortice.XAudio2;
using Vortice.XAudio2.Fx;

namespace XAudio2Sample
{
    class Program
    {
        const int XAUDIO2FX_DEBUG = 1;


        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr LoadLibrary(string dllToLoad);


        [DllImport("XAudio2SampleLibraryC.dll", EntryPoint = "CreateEffect")]
        private extern static int CreateEffectNative(IntPtr sourceVoice);

        [DllImport("XAudio2SampleLibraryC.dll", EntryPoint = "GetSize")]
        private extern static int GetSizeNative();

        [DllImport("XAudio2SampleLibraryC.dll", EntryPoint = "GetDataStruct")]
        private extern static ReverbParameters GetDataStructNative();


        static unsafe void Main(string[] args)
        {
            if (Environment.Is64BitProcess)
                LoadLibrary(@"x64\XAudio2SampleLibraryC.dll");
            else
                LoadLibrary(@"x86\XAudio2SampleLibraryC.dll");



            Console.WriteLine("Hello XAudio2!");

            var xaudio2 = XAudio2.XAudio2Create(ProcessorSpecifier.UseDefaultProcessor);

            DebugConfiguration debug = default;
            debug.TraceMask = 0x0001 | 0x0002;
            xaudio2.SetDebugConfiguration(debug, (IntPtr)0);

            var mv = xaudio2.CreateMasteringVoice(1, 48000);


            var voiceDetails = mv.GetVoiceDetails();

            var sampleRate = voiceDetails.InputSampleRate;
            var channels = voiceDetails.InputChannelCount;
            var channelMask = mv.ChannelMask;


            var reader = new NAudio.Wave.WaveFileReader("Test.wav");
            var array = new byte[reader.Length];
            reader.Read(array, 0, array.Length);
            var targetBuffer = new AudioBuffer(array);
            var encoding = reader.WaveFormat.Encoding;

            var waveFormat = WaveFormat.CreateCustomFormat((WaveFormatEncoding)encoding,
                reader.WaveFormat.SampleRate,
                reader.WaveFormat.Channels,
                reader.WaveFormat.AverageBytesPerSecond,
                reader.WaveFormat.BlockAlign,
                reader.WaveFormat.BitsPerSample);

            targetBuffer.LoopCount = IXAudio2.LoopInfinite;


            var sourceVoice = xaudio2.CreateSourceVoice(waveFormat);

            sourceVoice.SubmitSourceBuffer(targetBuffer);


            var reverb = Vortice.XAudio2.Fx.Fx.CreateAudioReverb();


            //var ef = new EffectDescriptor_Native
            //{
            //    EffectPointer = reverb.NativePointer,
            //    OutputChannelCount = 1,
            //    InitialState = true,
            //};

            //var ec = new EffectChain
            //{
            //    EffectDescriptorPointer = (IntPtr)(&ef),
            //    EffectCount = 1,
            //};

            //sourceVoice.SetEffectChain(ec);


            var ef = new EffectDescriptor(reverb, 1);

            sourceVoice.SetEffectChain(ef);


            var native = Vortice.XAudio2.Fx.Fx.ReverbConvertI3DL2ToNative(Vortice.XAudio2.Fx.Presets.Cave);

            sourceVoice.SetEffectParameters(0, native).CheckError();

            sourceVoice.EnableEffect(0);


            sourceVoice.Start();


            Console.WriteLine("Vortice XAudio2 Playing ...");
            Console.ReadKey();
        }
    }
}
