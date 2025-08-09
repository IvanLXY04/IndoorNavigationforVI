#pragma once
#include <string>
#include <vector>

// Initializes the speech recognition engine with a specific list of words to listen for.
// Call this once at the start of the program.
void InitializeSpeechRecognition(const std::vector<std::string>& words);

// Activates the microphone, listens for a single utterance, and returns the recognized text.
// This is a blocking call; it will wait until it hears something or times out.
std::string RecognizeDestination();

// Releases the speech recognition resources. Call this once before the program exits.
void CleanupSpeechRecognition();