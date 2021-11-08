#pragma once

#include "resource.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

class FListener {
public: void startListen(LPTSTR path);
};

void NotifyDirectory(LPTSTR);
