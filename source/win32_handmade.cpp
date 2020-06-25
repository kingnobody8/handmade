/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

   /*
	 TODO(casey):  THIS IS NOT A FINAL PLATFORM LAYER!!!

	 - Saved game locations
	 - Getting a handle to our own executable file
	 - Asset loading path
	 - Threading (launch a thread)
	 - Raw Input (support for multiple keyboards)
	 - Sleep/timeBeginPeriod
	 - ClipCursor() (for multimonitor support)
	 - Fullscreen support
	 - WM_SETCURSOR (control cursor visibility)
	 - QueryCancelAutoplay
	 - WM_ACTIVATEAPP (for when we are not the active application)
	 - Blit speed improvements (BitBlt)
	 - Hardware acceleration (OpenGL or Direct3D or BOTH??)
	 - GetKeyboardLayout (for French keyboards, international WASD support)

	 Just a partial list of stuff!!
   */


   // TODO(casey): Implement sine ourselves
#include <math.h>
#include <stdint.h>


#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#include "handmade.h"
#include "handmade.cpp"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>
#include <gl/gl.h>
global_variable GLuint GlobalBlitTextureHandle;


struct win32_offscreen_buffer
{
	// NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

internal void Win32InitOpenGL(HWND Window)
{
	HDC WindowDC = GetDC(Window);

	// TODO(casey): Hey Raymond Chen - what's the deal here?
	// Is cColorBits ACTUALLY supposed to exclude the alpha bits, like MSDN says, or not?
	PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
	DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
	DesiredPixelFormat.nVersion = 1;
	DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	DesiredPixelFormat.cColorBits = 32;
	DesiredPixelFormat.cAlphaBits = 8;
	DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
	PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
	DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
		sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
	SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

	HGLRC OpenGLRC = wglCreateContext(WindowDC);
	if (wglMakeCurrent(WindowDC, OpenGLRC))
	{
		glGenTextures(1, &GlobalBlitTextureHandle);
		// NOTE(casey): Success!!!
	}
	else
	{
		// InvalidCodePath;
		  // TODO(casey): Diagnostic
	}
	ReleaseDC(Window, WindowDC);
}

// TODO(casey): This is a global for now.
global_variable bool32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

// NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

void* PlatformLoadFile(char* Filename)
{
	// NOTE (casey): Implemens the Win32 file loading
	return(0);
}

internal void
Win32LoadXInput(void)
{
	// TODO(casey): Test this on Windows 8
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!XInputLibrary)
	{
		// TODO(casey): Diagnostic
		XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}

	if (!XInputLibrary)
	{
		// TODO(casey): Diagnostic
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}

	if (XInputLibrary)
	{
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) { XInputGetState = XInputGetStateStub; }

		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateStub; }

		// TODO(casey): Diagnostic

	}
	else
	{
		// TODO(casey): Diagnostic
	}
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
	// NOTE(casey): Load the library
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if (DSoundLibrary)
	{
		// NOTE(casey): Get a DirectSound object! - cooperative
		direct_sound_create* DirectSoundCreate = (direct_sound_create*)
			GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		// TODO(casey): Double-check that this works on XP - DirectSound8 or 7??
		LPDIRECTSOUND DirectSound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
		{
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
			WaveFormat.cbSize = 0;

			if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

				// NOTE(casey): "Create" a primary buffer
				// TODO(casey): DSBCAPS_GLOBALFOCUS?
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
				{
					HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
					if (SUCCEEDED(Error))
					{
						// NOTE(casey): We have finally set the format!
						OutputDebugStringA("Primary buffer format was set.\n");
					}
					else
					{
						// TODO(casey): Diagnostic
					}
				}
				else
				{
					// TODO(casey): Diagnostic
				}
			}
			else
			{
				// TODO(casey): Diagnostic
			}

			// TODO(casey): DSBCAPS_GETCURRENTPOSITION2
			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = 0;
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;
			HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
			if (SUCCEEDED(Error))
			{
				OutputDebugStringA("Secondary buffer created successfully.\n");
			}
		}
		else
		{
			// TODO(casey): Diagnostic
		}
	}
	else
	{
		// TODO(casey): Diagnostic
	}
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset)
{
	// TODO(casey): Let's see what the optimizer does

	uint8* Row = (uint8*)Buffer->Memory;
	for (int Y = 0;
		Y < Buffer->Height;
		++Y)
	{
		uint32* Pixel = (uint32*)Row;
		for (int X = 0;
			X < Buffer->Width;
			++X)
		{
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}

		Row += Buffer->Pitch;
	}
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height)
{
	// TODO(casey): Bulletproof this.
	// Maybe don't free first, free after, then free first if that fails.

	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;

	int BytesPerPixel = 4;

	// NOTE(casey): When the biHeight field is negative, this is the clue to
	// Windows to treat this bitmap as top-down, not bottom-up, meaning that
	// the first three bytes of the image are the color for the top left pixel
	// in the bitmap, not the bottom left!
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	// NOTE(casey): Thank you to Chris Hecker of Spy Party fame
	// for clarifying the deal with StretchDIBits and BitBlt!
	// No more DC for us.
	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * BytesPerPixel;

	// TODO(casey): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer* Buffer,
	HDC DeviceContext, int WindowWidth, int WindowHeight)
{
#if 0
	// TODO(casey): Aspect ratio correction
	// TODO(casey): Play with stretch modes
	StretchDIBits(DeviceContext,
		/*
		X, Y, Width, Height,
		X, Y, Width, Height,
		*/
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
#endif

	glViewport(0, 0, WindowWidth, WindowHeight);

	glBindTexture(GL_TEXTURE_2D, GlobalBlitTextureHandle);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Buffer->Width, Buffer->Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Buffer->Memory);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);

	glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);


	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glBegin(GL_TRIANGLES);

	// r32 P = 1.0f;
	float P = 1.0f;

	// NOTE(casey): Lower triangle
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-P, -P);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(P, -P);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(P, P);

	// NOTE(casey): Upper triangle
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-P, -P);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(P, P);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-P, P);

	glEnd();



	SwapBuffers(DeviceContext);

}

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
	case WM_CLOSE:
	{
		// TODO(casey): Handle this with a message to the user?
		GlobalRunning = false;
	} break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	} break;

	case WM_DESTROY:
	{
		// TODO(casey): Handle this as an error - recreate window?
		GlobalRunning = false;
	} break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 VKCode = WParam;
		bool32 WasDown = ((LParam & (1 << 30)) != 0);
		bool32 IsDown = ((LParam & (1 << 31)) == 0);
		if (WasDown != IsDown)
		{
			if (VKCode == 'W')
			{
			}
			else if (VKCode == 'A')
			{
			}
			else if (VKCode == 'S')
			{
			}
			else if (VKCode == 'D')
			{
			}
			else if (VKCode == 'Q')
			{
			}
			else if (VKCode == 'E')
			{
			}
			else if (VKCode == VK_UP)
			{
			}
			else if (VKCode == VK_LEFT)
			{
			}
			else if (VKCode == VK_DOWN)
			{
			}
			else if (VKCode == VK_RIGHT)
			{
			}
			else if (VKCode == VK_ESCAPE)
			{
				OutputDebugStringA("ESCAPE: ");
				if (IsDown)
				{
					OutputDebugStringA("IsDown ");
				}
				if (WasDown)
				{
					OutputDebugStringA("WasDown");
				}
				OutputDebugStringA("\n");
			}
			else if (VKCode == VK_SPACE)
			{
			}
		}

		bool32 AltKeyWasDown = (LParam & (1 << 29));
		if ((VKCode == VK_F4) && AltKeyWasDown)
		{
			GlobalRunning = false;
		}
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		win32_window_dimension Dimension = Win32GetWindowDimension(Window);
		Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
			Dimension.Width, Dimension.Height);
		EndPaint(Window, &Paint);
	} break;

	default:
	{
		//            OutputDebugStringA("default\n");
		Result = DefWindowProcA(Window, Message, WParam, LParam);
	} break;
	}

	return(Result);
}

struct win32_sound_output
{
	int SamplesPerSecond;
	int ToneHz;
	int16 ToneVolume;
	uint32 RunningSampleIndex;
	int WavePeriod;
	int BytesPerSample;
	int SecondaryBufferSize;
	real32 tSine;
	int LatencySampleCount;
};

internal void Win32ClearBuffer(win32_sound_output* SoundOutput)
{
	VOID* Region1;
	DWORD Region1Size;
	VOID* Region2;
	DWORD Region2Size;
	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
		&Region1, &Region1Size,
		&Region2, &Region2Size,
		0)))
	{
		// TODO(casey): assert that Region1Size/Region2Size is valid
		uint8* DestSample = (uint8*)Region1;
		for (DWORD ByteIndex = 0;
			ByteIndex < Region1Size;
			++ByteIndex)
		{
			*DestSample++ = 0;
		}

		DestSample = (uint8*)Region2;
		for (DWORD ByteIndex = 0;
			ByteIndex < Region2Size;
			++ByteIndex)
		{
			*DestSample++ = 0;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}


internal void
Win32FillSoundBuffer(win32_sound_output* SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer* SourceBuffer)
{
	// TODO(casey): More strenuous test!
	// TODO(casey): Switch to a sine wave
	VOID* Region1;
	DWORD Region1Size;
	VOID* Region2;
	DWORD Region2Size;
	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
		&Region1, &Region1Size,
		&Region2, &Region2Size,
		0)))
	{
		// TODO(casey): assert that Region1Size/Region2Size is valid

		// TODO(casey): Collapse these two loops
		DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
		int16* DestSample = (int16*)Region1;
		int16* SourceSample = (int16*)SourceBuffer->Samples;
		for (DWORD SampleIndex = 0;
			SampleIndex < Region1SampleCount;
			++SampleIndex)
		{
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}

		DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
		DestSample = (int16*)Region2;
		for (DWORD SampleIndex = 0;
			SampleIndex < Region2SampleCount;
			++SampleIndex)
		{
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

int CALLBACK
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
	Win32LoadXInput();

	WNDCLASSA WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	//    WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClassA(&WindowClass))
	{
		HWND Window =
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Handmade Hero",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if (Window)
		{
			// NOTE(casey): Since we specified CS_OWNDC, we can just
			// get one device context and use it forever because we
			// are not sharing it with anyone.
			HDC DeviceContext = GetDC(Window);

			Win32InitOpenGL(Window);
			// NOTE(casey): Graphics test
			int XOffset = 0;
			int YOffset = 0;

			win32_sound_output SoundOutput = {};

			// TODO(casey): Make this like sixty seconds?
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.ToneHz = 256;
			SoundOutput.ToneVolume = 3000;
			SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
			SoundOutput.BytesPerSample = sizeof(int16) * 2;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
			SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
			Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
			Win32ClearBuffer(&SoundOutput);
			//			Win32FillSoundBuffer(&SoundOutput, 0, SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample);
			GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

			GlobalRunning = true;

			int16* Samples = (int16*)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			while (GlobalRunning)
			{
				MSG Message;

				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}

					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				// TODO(casey): Should we poll this more frequently
				for (DWORD ControllerIndex = 0;
					ControllerIndex < XUSER_MAX_COUNT;
					++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						// NOTE(casey): This controller is plugged in
						// TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
						XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

						bool32 Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool32 Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool32 Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool32 Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool32 Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool32 Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool32 LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool32 RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool32 AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool32 BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool32 XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool32 YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;

						// TODO(casey): We will do deadzone handling later using
						// XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
						// XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689

						XOffset += StickX / 4096;
						YOffset += StickY / 4096;

						SoundOutput.ToneHz = 512 + (int)(256.0f * ((real32)StickY / 30000.0f));
						SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
					}
					else
					{
						// NOTE(casey): The controller is not available
					}
				}

				RenderWeirdGradient(&GlobalBackbuffer, XOffset, YOffset);

#if 1
				// NOTE(casey): DirectSound output test
				DWORD ByteToLock = 0;
				DWORD TargetCursor = 0;
				DWORD BytesToWrite = 0;
				DWORD PlayCursor = 0;
				DWORD WriteCursor = 0;
				bool32 SoundIsValid = false;
				// TODO(casey): Tighten up sound logic so that we know where we should be
				// writing to and can anticipate the time spent in the game update.

				if (SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
				{
					//DWORD ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);
					//DWORD TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);

					ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);
					TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);


					//TODO(casey): change this to using a lower latency offset from playcursor
					//when we actually start having sfx
					if (ByteToLock > TargetCursor)
					{
						BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
						BytesToWrite += TargetCursor;
					}
					else
					{
						BytesToWrite = TargetCursor - ByteToLock;
					}

					// TODO(casey): More strenuous test!
					// TODO(casey): Switch to a sine wave

					SoundIsValid = true;
				}



				game_sound_output_buffer SoundBuffer = {};
				SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
				SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
				SoundBuffer.Samples = Samples;

				game_offscreen_buffer Buffer = {};
				Buffer.Memory = GlobalBackbuffer.Memory;
				Buffer.Width = GlobalBackbuffer.Width;
				Buffer.Height = GlobalBackbuffer.Height;
				Buffer.Pitch = GlobalBackbuffer.Pitch;
				GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);

				// NOTE(casey): DirectSound output test
				if (SoundIsValid)
				{
					Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
				}


#endif
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
					Dimension.Width, Dimension.Height);
			}
		}
		else
		{
			// TODO(casey): Logging
		}
	}
	else
	{
		// TODO(casey): Logging
	}

	return(0);
}
