🗺️ Indoor Navigation System for the Visually Impaired 🧑‍🦯
- This project is a real-time C++ application that provides indoor navigation assistance for visually impaired (VI) individuals. The system uses a standard webcam to detect colored QR codes, determines the user's current location, and provides accessible voice-guided directions to a desired destination within a pre-mapped building.

✨ Core Features
- 👁️ Real-time Color QR Code Detection: A robust computer vision pipeline automatically finds and decodes red, green, or blue QR codes from a live webcam feed, even under varying lighting conditions and at different angles.
- 🧠 Intelligent Route Planning: Uses Dijkstra's algorithm to compute the mathematically shortest path from the user's current location to their destination, based on a pre-defined graph of the building's layout.
- 🔊 Accessible Audio Feedback: Leverages the Windows SAPI Text-to-Speech (TTS) engine to provide clear, turn-by-turn voice instructions. Includes a high-priority speech function to interrupt old messages and deliver critical updates instantly.
- 🎙️ Voice Command Control: Utilizes the Windows SAPI Speech-to-Text (STT) engine to allow users to set their destination hands-free, making the system highly accessible.

💻 Technology Stack
- Language: C++ (C++17 standard)
- IDE: Visual Studio 2022
- Core Library: OpenCV 4.x (for all computer vision tasks)
- Accessibility: Windows Speech API (SAPI) (for both TTS and STT)

🏗️ System Architecture
- The project is designed with a modular architecture to ensure clean separation of concerns:
  - main.cpp: The central controller that manages the main application loop and coordinates modules.
  - qr_detection.cpp: Handles the computer vision pipeline for finding and isolating QR codes.
  - qr_reader.cpp: Decodes the isolated QR code image into a location string.
  - route_guidance.cpp: Implements the building map as a graph and runs Dijkstra's algorithm.
  - audio_feedback.cpp: Manages all Text-to-Speech (TTS) output.
  - speech_recognition.cpp: Manages all Speech-to-Text (STT) input.
  - ui_vi.cpp: Manages the console-based user menu and input.

🤖 Key Algorithms & Vision Pipeline
The system's reliability stems from its sophisticated computer vision pipeline:
1. Frame Pre-processing:
  medianBlur() is applied to reduce sensor noise.
  The frame is converted to the HSV color space for reliable color detection.
  equalizeHist() is applied to the Value (V) channel to normalize for lighting variations.
2. Robust QR Code Detection (findAndWarpQRCode):
  A combined color mask (red, green, blue) is generated.
  findContours() with RETR_TREE is used to find shapes with a parent-child hierarchy.
  The code specifically identifies a QR code by finding a parent contour with at least three squarish child contours (the finder patterns).
  warpPerspective() is used to transform the detected QR code into a straightened, 200x200 image, correcting for any perspective distortion.
3. Stable Decoding (readQRCode):
  The straightened image is converted to grayscale and then to a high-contrast binary image using THRESH_OTSU.
  OpenCV's detectAndDecode() is called on this clean binary image for a high success rate.

⚙️ Getting Started
- To get a local copy up and running, follow these simple steps.

*Prerequisites*
- Visual Studio 2022 with C++ development workload.
- OpenCV 4.x: You must have OpenCV installed and configured correctly on your system.
- Windows 10/11 SDK: Required for the Speech API (SAPI).

*Installation*
1. Clone the repo:
  git clone https://github.com/IvanLXY04/IndoorNavigationForTheVisuallyImpaired-VI-usingQRcode.git
2. Open in Visual Studio:
  Open the .sln file in Visual Studio.
3. Configure OpenCV:
  Go to Project Properties -> VC++ Directories.
  Add the path to your OpenCV build/include folder in Include Directories.
  Add the path to your OpenCV build/x64/vc15/lib folder in Library Directories.
  Go to Linker -> Input and add opencv_world4xx.lib and opencv_world4xxd.lib to Additional Dependencies.
4. Build and Run:
  Compile the solution (Build -> Build Solution).
  Run the application (Debug -> Start Debugging).

🎮 How to Use
- The application is controlled via a simple, voice-guided console menu. Upon launching, you will be presented with the following options:
  1. Start Navigation (Random Destination): Initiates QR code scanning to find your start location and then guides you to a randomly selected destination.
  2. Where am I?: Scans a QR code and simply announces your current location.
  3. Set Destination by Voice: Prompts you to speak a destination (e.g., "N-zero-zero-eight"), then asks you to scan your starting QR code to begin navigation.
  4. Help: Provides help information.
  5. Exit: Closes the application.

🚀 Future Improvements
- Port the system to a mobile platform (Android/iOS) for real-world portability.
- Implement the A* algorithm as an alternative to Dijkstra's for improved performance on larger maps.
- Develop a more advanced UI instead of a simple console menu.
- Integrate real-time obstacle avoidance using depth sensors or additional computer vision techniques.

📜 License
- Distributed under the MIT License. See LICENSE for more information.

🙏 Acknowledgments
- This project was developed as part of the UCCC2513 Mini Project assignment.
- The OpenCV community
