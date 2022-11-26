#include "traffic_light_recognition.h"
#include "opencv2/opencv.hpp"

void test(std::string file_name) {
    cv::Mat img = cv::imread(file_name, -1);
    if (img.data == NULL) {
        std::cerr << "cannot open image" << std::endl;
        return;
    }
    hls::stream<in_t> src;
    std::vector<cv::Mat> src_channels;
    in_t src_;
    cv::split(img, src_channels);
    for (int i = 0; i < img.rows; i++) {

        for (int j = 0; j < img.cols; j++) {
            // src_.keep = -1;
            // src_.strb = -1;
            ap_uint<32> pixel;
            pixel.range(23, 16) = src_channels[2].at<ap_uint8_t>(i, j);
            pixel.range(15, 8) = src_channels[1].at<ap_uint8_t>(i, j);
            pixel.range(7, 0) = src_channels[0].at<ap_uint8_t>(i, j);
            src_.last = (i == img.rows - 1 && j == img.cols - 1);
            src_.data = pixel;
            src.write(src_);
        }
    }
    unsigned int color = 0;
    unsigned int cond_r[3] = { 200, 150, 150 };
    unsigned int cond_y[3] = { 200, 150, 150 };
    unsigned int cond_g[3] = { 150, 200, 150 };
    unsigned int circle_center_x, circle_center_y, circle_radius;
    void traffic_light_recognition(hls::stream<in_t> &stream_in, unsigned int rows, unsigned int cols, unsigned int radius,
            unsigned int cond_green[3], unsigned int cond_yellow[3], unsigned int cond_red[3], unsigned int *circle_center_x,
            unsigned int *circle_center_y, unsigned int *circle_radius, unsigned int *color);
    traffic_light_recognition(src, img.rows, img.cols, 10, cond_g, cond_y, cond_r, &circle_center_x, &circle_center_y, &circle_radius, &color);

    std::cout << file_name << ": " << color << std::endl;
    std::cout << "center x" << ": " << circle_center_x << std::endl;
    std::cout << "center y" << ": " << circle_center_y << std::endl;
    std::cout << "radius" << ": " << circle_radius << std::endl;
    std::cout << "--------------------------------------" << std::endl;
}

int main() {
    std::string file_name[] = { "green_256_128.jpg", "yellow_256_128.jpg", "red_256_128.jpg", "green_128_64.jpg", "yellow_128_64.jpg",
            "red_128_64.jpg", "green_64_32.jpg", "yellow_64_32.jpg", "red_64_32.jpg" };
    for (int i = 0; i < 9; i++) {
        test(file_name[i]);
    }
    return 0;
}
