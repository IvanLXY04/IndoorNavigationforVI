#include "qr_detection.h"

QRCodeResult findAndWarpQRCode(const cv::Mat& frame, const cv::Mat& mask) {
    std::vector<std::vector<cv::Point>> contours;
    findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::sort(contours.begin(), contours.end(), [](const auto& a, const auto& b) {
        return contourArea(a) > contourArea(b);
        });
    for (const auto& contour : contours) {
        if (contourArea(contour) < 1000) { continue; }
        std::vector<cv::Point> approx;
        double peri = arcLength(contour, true);
        approxPolyDP(contour, approx, 0.04 * peri, true);
        if (approx.size() == 4) {
            QRCodeResult result;
            result.boundingBox = boundingRect(approx);
            result.pixelWidth = result.boundingBox.width;
            std::vector<cv::Point2f> dest_pts = { {0.0f, 0.0f}, {200.0f, 0.0f}, {200.0f, 200.0f}, {0.0f, 200.0f} };
            std::vector<cv::Point2f> src_pts;
            for (const auto& p : approx) src_pts.push_back(p);
            cv::Mat transform = getPerspectiveTransform(src_pts, dest_pts);
            warpPerspective(frame, result.warpedImage, transform, { 200, 200 });
            return result;
        }
    }
    return {};
}