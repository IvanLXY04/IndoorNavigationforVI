#include "audio_feedback.h"
#include <sapi.h>
#include <iostream>

// Global pointer to the SAPI voice interface
ISpVoice* pVoice = NULL;

void InitializeTTS() {
    // Initialize COM library for the current thread
    if (FAILED(CoInitialize(NULL))) {
        std::cerr << "Error: Could not initialize COM library for TTS." << std::endl;
        return;
    }

    // Create an instance of the SAPI SpVoice object
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);

    if (FAILED(hr)) {
        std::cerr << "Error: Could not create SAPI voice instance." << std::endl;
        pVoice = NULL;
    }
    else {
        std::cout << "Text-to-Speech engine initialized." << std::endl;
    }
}

void CleanupTTS() {
    if (pVoice) {
        pVoice->Release();
        pVoice = NULL;
    }
    CoUninitialize();
}

void Speak(const std::string& text) {
    if (pVoice) {
        // Convert the std::string to a wide string (WCHAR*), which SAPI requires.
        std::wstring wide_text(text.begin(), text.end());
        // SPF_ASYNC makes the call non-blocking. Use SPF_DEFAULT to make it blocking.
        pVoice->Speak(wide_text.c_str(), SPF_ASYNC, NULL);
    }
}

