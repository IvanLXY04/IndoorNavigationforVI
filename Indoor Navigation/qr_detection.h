#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// A structure to hold the result of a successful QR code detection
struct QRCodeResult {
    cv::Mat warpedImage;      // The straightened, flat image for decoding
    cv::Rect boundingBox;     // The original bounding box in the frame
    float pixelWidth = 0.0f;  // The width for distance estimation
    bool isValid() const { return !warpedImage.empty(); }
};

// Finds a potential QR code based on a color mask, corrects its perspective,
// and returns the result.
QRCodeResult findAndWarpQRCode(const cv::Mat& frame, const cv::Mat& mask);