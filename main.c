#include <stdio.h>
#include <SDL2/SDL.h>

// type annotations
typedef unsigned char         u8;
typedef signed char           i8;
typedef unsigned short        u16;
typedef short                 i16;
typedef unsigned int          u32;
typedef int                   i32;
typedef unsigned long long    u64;
typedef long long             i64;
typedef float                 f32; 
typedef double                f64;
typedef size_t                s64;
typedef void                  u0;

#define MAX_SAMPLES 2048
f32 buffer[MAX_SAMPLES];
i32 buffer_idx = 0;

f32 sine_wave(f32 time, f32 freq) {
   return sin(freq * 2.0f * M_PI * time);
}

f32 square_wave(f32 time, f32 freq) {
   return sin(freq * 2.0f * M_PI * time) >= 0.0 ? 1.0 : -1.0;
}

f32 sawtooth_wave(f32 time, f32 freq) {
   f32 period = 1.0f / freq;
   f32 t = fmod(time, period);
   return (2.0f * t / period) - 1.0f;
}

u0 audio_callback(u0 *userdata, u8 *stream, i32 len) {
   u64 *samples_played = (u64*)userdata;
   f32 *fstream = (f32*)(stream);

   static const f32 volume = 0.2;
   static const f32 frequency = 1000.0;
   
   for(i32 sid = 0; sid < (len / 8); ++sid) {
      f64 time = (*samples_played + sid) / 44100.0;
      f32 sample = volume * sine_wave(time, frequency);

      fstream[2 * sid + 0] = sample; // L 
      fstream[2 * sid + 1] = sample; // R 
      buffer[(buffer_idx + sid) % MAX_SAMPLES] = sample;
   }
   
   buffer_idx = (buffer_idx + (len / 8)) % MAX_SAMPLES;
   *samples_played += (len / 8);
}

i32 main(i32 argc, char **argv) {
   u64 samples_played = 0;

   if(SDL_Init(SDL_INIT_AUDIO) < 0) {
      fprintf(stderr, "Error initializing SDL. SDL_Error: %s\n", SDL_GetError());
      return -1;
   }

   SDL_AudioSpec audio_spec_want, audio_spec;
   SDL_memset(&audio_spec_want, 0, sizeof(audio_spec_want));

   audio_spec_want.freq     = 44100;
   audio_spec_want.format   = AUDIO_F32;
   audio_spec_want.channels = 2;
   audio_spec_want.samples  = 512;
   audio_spec_want.callback = audio_callback;
   audio_spec_want.userdata = (u0*)&samples_played;

   SDL_AudioDeviceID audio_device_id = SDL_OpenAudioDevice(
      NULL, 0,
      &audio_spec_want, &audio_spec,
      SDL_AUDIO_ALLOW_FORMAT_CHANGE
   );

   if(!audio_device_id)
   {
      fprintf(stderr, "Error creating SDL audio device. SDL_Error: %s\n", SDL_GetError());
      SDL_Quit();
      return -1;
   }

   i32 window_width  = 600;
   i32 window_height = 300;
   SDL_Window *window;
   SDL_Renderer *renderer;
   
   {
      window = SDL_CreateWindow(
         "Tone Analyzer",
         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
         window_width, window_height,
         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
      );

      if(!window) {
         fprintf(stderr, "Error creating SDL window. SDL_Error: %s\n", SDL_GetError());
         SDL_Quit();
         return -1;
      }

      renderer = SDL_CreateRenderer(
         window,
         -1,
         0
      );

      if (!renderer) {
         fprintf(stderr, "Error creating SDL renderer. SDL_Error: %s\n", SDL_GetError());
         SDL_Quit();
         return -1;
      }
   }

   SDL_PauseAudioDevice(audio_device_id, 0);
   u8 running = 1;
   while(running) {      
      SDL_Event sdl_event;
      while(SDL_PollEvent(&sdl_event) != 0) {
         if (sdl_event.type == SDL_QUIT)
            running = 0;
         }
         SDL_SetRenderDrawColor(renderer, 0xde, 0xad, 0xbe ,0xff);
         SDL_RenderClear(renderer);

         SDL_SetRenderDrawColor(renderer, 0,0,0,0xff);
         for (i32 x = 0; x < 600; x++) {
            int y = 150 + (buffer[x] * 150);
            SDL_RenderDrawPoint(renderer, x, y);
            // SDL_RenderDrawLine(renderer, x, 150, x, y);
         }
         SDL_RenderPresent(renderer);
      }
      
      
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   SDL_CloseAudioDevice(audio_device_id);
   SDL_Quit();

   return 0;
}