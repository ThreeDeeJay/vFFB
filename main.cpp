#include <iostream>
#include <windows.h>
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

LPDIRECTINPUT8       g_pDI = NULL;
LPDIRECTINPUTDEVICE8 g_pFFDevice = NULL;

BOOL InitDirectInput(HINSTANCE hInstance);
VOID Cleanup();
BOOL InitForceFeedback();
VOID SetForceFeedbackEffect();

int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!InitDirectInput(hInstance)) {
        std::cerr << "Failed to initialize DirectInput." << std::endl;
        return 1;
    }

    if (!InitForceFeedback()) {
        std::cerr << "Failed to initialize force feedback." << std::endl;
        Cleanup();
        return 1;
    }

    SetForceFeedbackEffect();

    // Keep the application running to maintain the force feedback effect
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    Cleanup();
    return 0;
}

BOOL InitDirectInput(HINSTANCE hInstance) {
    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&g_pDI, NULL))) {
        return FALSE;
    }
    return TRUE;
}

VOID Cleanup() {
    if (g_pFFDevice) {
        g_pFFDevice->Unacquire();
        g_pFFDevice->Release();
        g_pFFDevice = NULL;
    }

    if (g_pDI) {
        g_pDI->Release();
        g_pDI = NULL;
    }
}

BOOL InitForceFeedback() {
    if (FAILED(g_pDI->CreateDevice(GUID_Joystick, &g_pFFDevice, NULL))) {
        return FALSE;
    }

    if (FAILED(g_pFFDevice->SetDataFormat(&c_dfDIJoystick))) {
        return FALSE;
    }

    if (FAILED(g_pFFDevice->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND))) {
        return FALSE;
    }

    if (FAILED(g_pFFDevice->Acquire())) {
        return FALSE;
    }

    if (FAILED(g_pFFDevice->SendForceFeedbackCommand(DISFFC_RESET))) {
        return FALSE;
    }

    return TRUE;
}

VOID SetForceFeedbackEffect() {
    DICONSTANTFORCE cf;
    cf.lMagnitude = DI_FFNOMINALMAX;

    DIEFFECT eff;
    ZeroMemory(&eff, sizeof(eff));
    eff.dwSize = sizeof(DIEFFECT);
    eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    eff.dwDuration = INFINITE;
    eff.dwSamplePeriod = 0;
    eff.dwGain = DI_FFNOMINALMAX;
    eff.dwTriggerButton = DIEB_NOTRIGGER;
    eff.dwTriggerRepeatInterval = 0;
    eff.cAxes = 1;
    eff.rgdwAxes = NULL;
    eff.rglDirection = NULL;
    eff.lpEnvelope = NULL;
    eff.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
    eff.lpvTypeSpecificParams = &cf;

    LPDIRECTINPUTEFFECT pEffect;
    if (FAILED(g_pFFDevice->CreateEffect(GUID_ConstantForce, &eff, &pEffect, NULL))) {
        std::cerr << "Failed to create force feedback effect." << std::endl;
        return;
    }

    if (FAILED(pEffect->Start(1, 0))) {
        std::cerr << "Failed to start force feedback effect." << std::endl;
        pEffect->Release();
        return;
    }
}