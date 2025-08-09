#include "qr_reader.h"

std::string readQRCode(const cv::Mat& frame) {
    cv::QRCodeDetector detector;
    std::string decoded = detector.detectAndDecode(frame);
    return decoded;
}