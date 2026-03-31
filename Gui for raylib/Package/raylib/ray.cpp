#include "ray.h"
#include "raylib.h"
#include "../win/win.h"
#include <stdio.h>

namespace raylib
{
	void* GetWindowHandle()
	{
		return (win::HWND)::GetWindowHandle();
	}

	bool FileExists_(const char* fileName) {
		return FileExists(fileName);
	}
}


