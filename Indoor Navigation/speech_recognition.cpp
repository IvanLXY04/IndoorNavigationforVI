#include "speech_recognition.h"
#include <sapi.h>
#include <sphelper.h>
#include <iostream>
#include <atlbase.h>

// --- Global COM Pointers for Speech Recognition ---
CComPtr<ISpRecognizer> cpRecognizer;
CComPtr<ISpRecoContext> cpRecoContext;
CComPtr<ISpRecoGrammar> cpGrammar;
CComPtr<ISpAudio> cpAudio;

void InitializeSpeechRecognition(const std::vector<std::string>& words) {
    if (FAILED(cpRecognizer.CoCreateInstance(CLSID_SpInprocRecognizer))) {
        std::cerr << "Error: Could not create speech recognizer instance." << std::endl;
        return;
    }
    if (FAILED(SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &cpAudio))) {
        std::cerr << "Error: Could not create default audio input object." << std::endl;
        return;
    }
    if (FAILED(cpRecognizer->SetInput(cpAudio, TRUE))) {
        std::cerr << "Error: Could not set audio input for recognizer." << std::endl;
        return;
    }
    if (FAILED(cpRecognizer->CreateRecoContext(&cpRecoContext))) {
        std::cerr << "Error: Could not create recognition context." << std::endl;
        return;
    }

    // Set the context to notify us when a recognition event occurs
    // This is crucial for the event handle to work.
    if (FAILED(cpRecoContext->SetNotifyWin32Event())) {
        std::cerr << "Error: Could not set Win32 event notification." << std::endl;
        return;
    }

    const ULONGLONG ullInterest = SPFEI(SPEI_RECOGNITION);
    if (FAILED(cpRecoContext->SetInterest(ullInterest, ullInterest))) {
        std::cerr << "Error: Could not set recognition context interest." << std::endl;
        return;
    }
    if (FAILED(cpRecoContext->CreateGrammar(0, &cpGrammar))) {
        std::cerr << "Error: Could not create grammar object." << std::endl;
        return;
    }
    SPSTATEHANDLE hState;
    if (FAILED(cpGrammar->GetRule(L"DestinationRule", 0, SPRAF_TopLevel | SPRAF_Active, TRUE, &hState))) {
        std::cerr << "Error: Could not get grammar rule." << std::endl;
        return;
    }
    for (const auto& word : words) {
        std::wstring wide_word(word.begin(), word.end());
        cpGrammar->AddWordTransition(hState, NULL, wide_word.c_str(), L" ", SPWT_LEXICAL, 1.0f, NULL);
    }
    if (FAILED(cpGrammar->Commit(0))) {
        std::cerr << "Error: Could not commit grammar changes." << std::endl;
        return;
    }
    std::cout << "Speech Recognition engine initialized." << std::endl;
}


// **CORRECTED: This function now uses the proper event handle waiting mechanism**
std::string RecognizeDestination() {
    if (!cpGrammar || !cpRecoContext) return "";

    if (FAILED(cpGrammar->SetRuleState(L"DestinationRule", NULL, SPRS_ACTIVE))) {
        std::cerr << "Error: Could not activate grammar rule." << std::endl;
        return "";
    }

    // --- The Correct Way to Wait for an Event with a Timeout ---
    // 1. Get the notification handle from the recognition context
    HANDLE hEvent = cpRecoContext->GetNotifyEventHandle();
    if (hEvent == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not get notification handle." << std::endl;
        return "";
    }

    // 2. Use the Windows API to wait on that handle
    // This will pause for up to 5 seconds (5000ms) or until a speech event occurs
    DWORD dwResult = WaitForSingleObject(hEvent, 5000);

    // 3. Check the result of the wait
    if (dwResult == WAIT_OBJECT_0) { // This means the event was signaled (speech happened)
        CSpEvent event;
        // 4. Now, get all events from the queue (there should be at least one)
        while (SUCCEEDED(event.GetFrom(cpRecoContext)) && event.eEventId != SPEI_END_SR_STREAM) {
            if (event.eEventId == SPEI_RECOGNITION) {
                ISpRecoResult* pResult = event.RecoResult();
                wchar_t* pszCoMemText = NULL;
                if (SUCCEEDED(pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pszCoMemText, NULL))) {
                    std::wstring wide_str(pszCoMemText);
                    CoTaskMemFree(pszCoMemText);
                    // Deactivate the rule to stop listening unnecessarily
                    cpGrammar->SetRuleState(L"DestinationRule", NULL, SPRS_INACTIVE);
                    return std::string(wide_str.begin(), wide_str.end());
                }
            }
        }
    }

    // If we get here, it's because the wait timed out or failed.
    cpGrammar->SetRuleState(L"DestinationRule", NULL, SPRS_INACTIVE);
    return "";
}


void CleanupSpeechRecognition() {
    // CComPtr handles the release automatically
}