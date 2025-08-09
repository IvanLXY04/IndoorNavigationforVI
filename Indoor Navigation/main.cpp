#include <opencv2/opencv.hpp>
#include <iostream>
#include <windows.h>
#include <map>
#include <ctime>
#include <chrono>
#include <filesystem>
#include <vector>

#include "qr_detection.h"
#include "qr_reader.h"
#include "route_guidance.h"
#include "ui_vi.h"
#include "audio_feedback.h"
#include "speech_recognition.h"

using namespace cv;
using namespace std;

// --- Constants for Distance Estimation ---
const float KNOWN_QR_WIDTH_CM = 15.0f;
const float FOCAL_LENGTH = 650.0f;

// High-precision coordinates for all nodes
map<string, Point> nodeCoordinates = {
    {"Right Corner of N001", Point(1600, 631)},
    {"N001", Point(1504, 622)},
    {"N002", Point(1373, 610)},
    {"N003", Point(1245, 598)},
    {"Main Entrance", Point(1189, 588)},
    {"Main Entrance Stair", Point(1147, 591)},
    {"The Olive Place", Point(1159, 653)},
    {"N004", Point(1004, 610)},
    {"N005", Point(874, 622)},
    {"N006", Point(747, 633)},
    {"Alley 1 in front N006", Point(755, 671)},
    {"N007", Point(622, 646)},
    {"Left Corner of N007", Point(564, 657)},
    {"N008", Point(562, 705)},
    {"Toilets Near N008", Point(755, 706)},
    {"N009", Point(801, 709)},
    {"N010", Point(964, 711)},
    {"Left Corner of N011", Point(1158, 704)},
    {"N011", Point(1180, 708)},
    {"N012", Point(1348, 712)},
    {"Toilets Near N012", Point(1587, 711)}
};

// List of nodes that can be chosen as a destination (for random mode and voice grammar)
vector<string> destinationNodes = {
    "N001", "N002", "N003", "Main Entrance", "Main Entrance Stair",
    "The Olive Place", "N004", "N005", "N006", "N007", "N008",
    "Toilets Near N008", "N009", "N010", "N011", "N012", "Toilets Near N012"
};

// === Function to compute vector and distance feedback ===
string computeDirectionFeedback(Point frameCenter, Point qrCenter, float qrPixelWidth) {
    float distance = (KNOWN_QR_WIDTH_CM * FOCAL_LENGTH) / qrPixelWidth;
    string dist_str = to_string((int)distance) + " centimeters away. ";

    int dx = qrCenter.x - frameCenter.x;
    if (abs(dx) < 50) return "Aligned. " + dist_str;
    if (dx > 50) return "Move camera to the right. " + dist_str;
    return "Move camera to the left. " + dist_str;
}

// === Draw route graphically on map ===
// Change the return type to bool
bool drawRouteOnMap(const vector<string>& path, const string& mapPath, const string& start, const string& end) {
    Mat mapImg = imread(mapPath);
    if (mapImg.empty()) {
        cerr << "FATAL ERROR: Could not load map image from path: " << mapPath << endl;
        Speak("Fatal error. Map image not found.");
        return false;
    }

    for (size_t i = 1; i < path.size(); ++i) {
        line(mapImg, nodeCoordinates.at(path[i - 1]), nodeCoordinates.at(path[i]), Scalar(0, 255, 255), 3);
    }

    Point startPt = nodeCoordinates.at(start);
    circle(mapImg, startPt, 10, Scalar(255, 200, 200), FILLED);
    putText(mapImg, "You are here: " + start, startPt + Point(15, 0), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 5);
    putText(mapImg, "You are here: " + start, startPt + Point(15, 0), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);

    Point endPt = nodeCoordinates.at(end);
    circle(mapImg, endPt, 10, Scalar(200, 200, 255), FILLED);
    putText(mapImg, "Destination: " + end, endPt + Point(15, 0), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 0), 5);
    putText(mapImg, "Destination: " + end, endPt + Point(15, 0), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);

    imshow("Floor Map Route", mapImg);
    waitKey(1);
    return true;
}

// === Load graph structure with corrected, logical pathing ===
void loadFICTMap(RoutePlanner& planner) {
    for (const auto& pair : nodeCoordinates) {
        planner.addNode(pair.first);
    }
    planner.addEdge("Right Corner of N001", "N001", 10);
    planner.addEdge("N001", "N002", 10);
    planner.addEdge("N002", "N003", 10);
    planner.addEdge("N003", "Main Entrance", 10);
    planner.addEdge("Main Entrance", "Main Entrance Stair", 10);
    planner.addEdge("Main Entrance Stair", "N004", 10);
    planner.addEdge("N004", "N005", 10);
    planner.addEdge("N005", "N006", 10);
    planner.addEdge("N006", "N007", 10);
    planner.addEdge("N007", "Left Corner of N007", 10);
    planner.addEdge("Left Corner of N007", "N008", 10);
    planner.addEdge("N008", "Toilets Near N008", 10);
    planner.addEdge("Toilets Near N008", "N009", 10);
    planner.addEdge("N009", "N010", 10);
    planner.addEdge("N010", "Left Corner of N011", 10);
    planner.addEdge("Left Corner of N011", "N011", 10);
    planner.addEdge("N011", "N012", 10);
    planner.addEdge("N012", "Toilets Near N012", 10);
    planner.addEdge("Toilets Near N012", "Right Corner of N001", 10);
    planner.addEdge("Main Entrance Stair", "The Olive Place", 10);
    planner.addEdge("N006", "Alley 1 in front N006", 10);
    planner.addEdge("Alley 1 in front N006", "The Olive Place", 10);
    planner.addEdge("Left Corner of N011", "The Olive Place", 10);
}

// === Generate descriptive spoken route instructions ===
string generateRouteNarration(const vector<string>& path) {
    if (path.size() < 2) {
        return "Cannot generate route. You may already be at your destination.";
    }
    string destination = path.back();
    string narration = "To get to your destination, " + destination + ". ";
    if (path.size() == 2) {
        narration += "You can proceed directly there.";
    }
    else {
        narration += "You will need to pass by ";
        for (size_t i = 1; i < path.size() - 1; ++i) {
            narration += path[i];
            if (i < path.size() - 2) {
                narration += ", then ";
            }
        }
        narration += ", before arriving at your final destination.";
    }
    return narration;
}

// === Scanning function with controlled feedback ===
string startScanningSequence(VideoCapture& cap) {
    Speak("Starting scanner. Please pan your camera around to find a QR code.");
    auto lastSpokenTime = chrono::steady_clock::now();
    const int SPEECH_DELAY_SECONDS = 3;
    map<string, pair<Scalar, Scalar>> colorRanges = {
        {"blue", {Scalar(100, 150, 50), Scalar(140, 255, 255)}},
        {"green", {Scalar(40, 70, 50), Scalar(80, 255, 255)}}
    };
    Mat frame, hsv;
    while (true) {
        cap >> frame;
        if (frame.empty()) { cerr << "Camera feed lost." << endl; Speak("Camera feed lost."); return ""; }
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        vector<Mat> hsv_channels;
        split(hsv, hsv_channels);
        equalizeHist(hsv_channels[2], hsv_channels[2]);
        merge(hsv_channels, hsv);
        Mat combined_mask;
        Mat mask_red1, mask_red2;
        inRange(hsv, Scalar(0, 120, 70), Scalar(10, 255, 255), mask_red1);
        inRange(hsv, Scalar(170, 120, 70), Scalar(179, 255, 255), mask_red2);
        combined_mask = mask_red1 | mask_red2;
        for (const auto& color_pair : colorRanges) { Mat mask; inRange(hsv, color_pair.second.first, color_pair.second.second, mask); combined_mask |= mask; }
        morphologyEx(combined_mask, combined_mask, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(10, 10)));
        QRCodeResult qrResult = findAndWarpQRCode(frame, combined_mask);
        Point frameCenter(frame.cols / 2, frame.rows / 2);
        if (qrResult.isValid()) {
            rectangle(frame, qrResult.boundingBox, Scalar(0, 255, 0), 3);
            auto currentTime = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::seconds>(currentTime - lastSpokenTime).count() >= SPEECH_DELAY_SECONDS) {
                string feedback = computeDirectionFeedback(frameCenter, qrResult.boundingBox.tl() + Point(qrResult.boundingBox.width / 2, qrResult.boundingBox.height / 2), qrResult.pixelWidth);
                putText(frame, feedback, Point(30, frame.rows - 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
                Speak(feedback);
                lastSpokenTime = currentTime;
            }
            if (qrResult.pixelWidth > 150) {
                string qrData = readQRCode(qrResult.warpedImage);
                if (!qrData.empty()) { cout << "\n✅ QR Code Decoded: " << qrData << endl; Speak("Location found. You are at " + qrData); destroyWindow("QR Scanner"); return qrData; }
            }
        }
        rectangle(frame, Rect(frameCenter - Point(100, 100), Size(200, 200)), Scalar(255, 255, 255), 2);
        putText(frame, "Align QR Code Here", frameCenter - Point(95, 110), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
        imshow("QR Scanner", frame);
        if (waitKey(50) == 27) { Speak("Scanning cancelled."); destroyWindow("QR Scanner"); return ""; }
    }
}

int main(int argc, char* argv[]) {
    // Robust Path Resolution for the map image
    std::filesystem::path exe_path = argv[0];
    std::filesystem::path map_dir = exe_path.parent_path();
    std::string map_path_str = (map_dir / "FICT floor map.jpg").string();
    cout << "Attempting to load map from: " << map_path_str << endl;

    // Initialize services
    InitializeTTS();
    InitializeSpeechRecognition(destinationNodes);
    srand(time(0));

    // Initialize hardware
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "FATAL ERROR: Camera not found." << endl;
        Speak("Error. Camera not found.");
        CleanupTTS();
        return -1;
    }

    // Initialize data structures
    RoutePlanner planner;
    loadFICTMap(planner);
    string currentLocation = "";
    string destination = "";
    vector<string> currentPath;

    // Main application loop
    while (true) {
        showMainMenu();
        int choice = getUserChoice();

        if (choice == 1) { // Start Navigation (Random Destination)
            currentLocation = startScanningSequence(cap);
            if (!currentLocation.empty()) {
                do {
                    destination = destinationNodes[rand() % destinationNodes.size()];
                } while (destination == currentLocation);

                cout << "\n🎯 Random Destination Assigned: " << destination << endl;
                Speak("Your destination is " + destination);
                currentPath = planner.computeRoute(currentLocation, destination);

                if (currentPath.size() < 2) {
                    Speak("Could not compute a route.");
                }
                else {
                    string narration = generateRouteNarration(currentPath);
                    Speak(narration);
                    bool mapShown = drawRouteOnMap(currentPath, map_path_str, currentLocation, destination);
                    if (mapShown) {
                        Speak("The visual map is now displayed. Press any key on the map window to close it.");
                        waitKey(0);
                        try {
                            destroyWindow("Floor Map Route");
                        } catch (const cv::Exception& e) {
                            cerr << "Warning: Could not destroy window. " << e.what() << endl;
                        }
                    }
                }
            }
        }
        else if (choice == 2) { // Where am I?
            currentLocation = startScanningSequence(cap);
        }
        else if (choice == 3) { // Set Destination by Voice
            Speak("Please say your desired destination now.");
            string spokenDest = RecognizeDestination();

            if (spokenDest.empty()) {
                Speak("Sorry, I could not understand you. Please try again from the main menu.");
            }
            else {
                Speak("I heard " + spokenDest + ". Is this correct? Please scan your current location to confirm.");

                currentLocation = startScanningSequence(cap);

                // **THE FIX IS HERE**
                // If the user cancelled the scan, just go back to the main menu
                if (currentLocation.empty()) {
                    cout << "\nScan cancelled. Returning to main menu." << endl;
                    continue; // Go to the next iteration of the while(true) loop
                }

                // If scan was successful, proceed
                if (currentLocation == spokenDest) {
                    Speak("You are already at your destination.");
                    continue;
                }
                destination = spokenDest;
                cout << "\n🎯 Voice Destination Set: " << destination << endl;

                currentPath = planner.computeRoute(currentLocation, destination);

                if (currentPath.size() < 2) {
                    Speak("Could not compute a route to the spoken destination.");
                }
                else {
                    string narration = generateRouteNarration(currentPath);
                    Speak(narration);
                    drawRouteOnMap(currentPath, map_path_str, currentLocation, destination);
                    Speak("The visual map is now displayed. Press any key on the map window to close it.");
                    waitKey(0);
                    try {
                        destroyWindow("Floor Map Route");
                    } catch (const cv::Exception& e) {
                        cerr << "Warning: Could not destroy window. " << e.what() << endl;
                    }
                }
            }
        }
        else if (choice == 4) { // Help
            Speak("To use this system, select an option. Option 1 will find your location and give you a random destination. Option 3 allows you to speak your destination. Please speak clearly after the prompt. Option 5 will exit.");
        }
        else if (choice == 5) { // Exit
            Speak("Exiting navigation system. Goodbye.");
            break;
        }
    }

    // Cleanup resources
    cap.release();
    destroyAllWindows();
    CleanupSpeechRecognition();
    CleanupTTS();
    return 0;
}