#include "traffic_light_recognition.h"
//#define DEBUG

bool condition_green(ap_uint8_t r, ap_uint8_t g, ap_uint8_t b, cond_t cond_g) {
    return (r < cond_g.r) && (g > cond_g.g) && (b > cond_g.b);
}

bool condition_yellow(ap_uint8_t r, ap_uint8_t g, ap_uint8_t b, cond_t cond_y) {
    return (r > cond_y.r) && (g > cond_y.g) && (b < cond_y.b);
}

bool condition_red(ap_uint8_t r, ap_uint8_t g, ap_uint8_t b, cond_t cond_r) {
    return (r > cond_r.r) && (g < cond_r.g) && (b < cond_r.b);
}

bool is_candidate_region(ap_uint8_t r, ap_uint8_t g, ap_uint8_t b, cond_t cond_g, cond_t cond_y, cond_t cond_r) {
    return condition_green(r, g, b, cond_g) || condition_yellow(r, g, b, cond_y) || condition_red(r, g, b, cond_r);
}

void extract_candidate_regions(hls::stream<in_t> &stream_in, Image<HEIGHT, WIDTH, ap_uint<DATA_WIDTH_IN>> &candidate_regions_img,
        cond_t cond_g, cond_t cond_y, cond_t cond_r) {
    const int scale = 500;
    for (int i = 0; i < candidate_regions_img.rows; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=128
        for (int j = 0; j < candidate_regions_img.cols; j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=256
            ap_uint<DATA_WIDTH_IN> pixel = stream_in.read().data.to_uint();
            ap_uint8_t r = pixel.range(23, 16).to_uint();
            ap_uint8_t g = pixel.range(15, 8).to_uint();
            ap_uint8_t b = pixel.range(7, 0).to_uint();
            float s = float(b + g + r);
            if (s == 0.0f) {
                pixel = 0;
            } else {
                ap_uint8_t norm_r = (ap_uint8_t) (scale * r / s);
                ap_uint8_t norm_g = (ap_uint8_t) (scale * g / s);
                ap_uint8_t norm_b = (ap_uint8_t) (scale * b / s);
                if (!is_candidate_region(norm_r, norm_g, norm_b, cond_g, cond_y, cond_r)) {
                    pixel = 0;
                }
            }
            candidate_regions_img.val[i][j] = pixel;
        }
    }
}

void hough_vote(Image<HEIGHT, WIDTH, ap_uint<DATA_WIDTH_IN>> &candidate_regions_img, int radius, struct Circle &circle) {
    int max_total_votes = 0;
    LOOPN: for (int n = 0; n < 180; n += 1) {
        float r_cos_n = radius * cos_n[n];
        float r_sin_n = radius * sin_n[n];
#pragma HLS LOOP_TRIPCOUNT min=1 max=360
        LOOPI: for (int i = radius; i < candidate_regions_img.rows; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=128
            LOOPJ: for (int j = radius; j < candidate_regions_img.cols; j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=256
#pragma HLS pipeline II=2
                int votes = 0;
                if (candidate_regions_img.val[i][j] == 0) {
                    continue;
                }
                int a = int(i - (r_cos_n)); // cos
                int b = int(j - (r_sin_n));
                bool nearest_1_1 = false;
                bool nearest_1_2 = false;
                bool nearest_1_3 = false;
                bool nearest_1_4 = false;

                bool nearest_2_1 = false;
                bool nearest_2_2 = false;
                bool nearest_2_3 = false;
                bool nearest_2_4 = false;
                if (b - 1 > 0 && a - 1 > 0 && a + 1 < candidate_regions_img.rows && b + 1 < candidate_regions_img.cols) {
                    nearest_1_1 = candidate_regions_img.val[a][b + 1] > 0;
                    nearest_1_2 = candidate_regions_img.val[a][b - 1] > 0;
                    nearest_1_3 = candidate_regions_img.val[a + 1][b] > 0;
                    nearest_1_4 = candidate_regions_img.val[a - 1][b] > 0;
                }

                int a_2 = int(i - (-r_cos_n));
                int b_2 = int(j - (-r_cos_n));
                if (b_2 - 1 > 0 && a_2 - 1 > 0 && a_2 + 1 < candidate_regions_img.rows && b_2 + 1 < candidate_regions_img.cols) {
                    nearest_2_1 = candidate_regions_img.val[a_2][b_2 + 1] > 0;
                    nearest_2_2 = candidate_regions_img.val[a_2][b_2 - 1] > 0;
                    nearest_2_3 = candidate_regions_img.val[a_2 + 1][b_2] > 0;
                    nearest_2_4 = candidate_regions_img.val[a_2 - 1][b_2] > 0;
                }
                bool nearest_or_1_2 = (nearest_1_1 && nearest_1_2 && nearest_1_3 && nearest_1_4)
                        || (nearest_2_1 && nearest_2_2 && nearest_2_3 && nearest_2_4);
                bool nearest_and_1_2 = (nearest_1_1 && nearest_1_2 && nearest_1_3 && nearest_1_4)
                        && (nearest_2_1 && nearest_2_2 && nearest_2_3 && nearest_2_4);
                if (nearest_and_1_2) {
                    votes += 2;
                } else if (nearest_or_1_2) {
                    votes += 1;
                }
                if (votes > max_total_votes) {
                    circle.center_y = i;
                    circle.center_x = j;
                    circle.radius = radius;
                    max_total_votes = votes;
                }
            }
        }
    }
}

void hough_circle(Image<HEIGHT, WIDTH, ap_uint<DATA_WIDTH_IN>> &candidate_regions_img, unsigned int radius, struct Circle &circle) {
#if defined(DEBUG)
    std::cout << "Start hough_circle()" << std::endl;
    std::cout << "search by radius " << radius << std::endl;
#endif
    circle.radius = 0;
    circle.center_x = 0;
    circle.center_y = 0;
    hough_vote(candidate_regions_img, radius, circle);
#if defined(DEBUG)
        std::cout << "Search for maximum voting when circle detected with radius " << radius
                << ", loop size : height = " << ballot_box.rows << ", width = " << ballot_box.cols << std::endl;
        std::cout << "loop size: " << "height = " << ballot_box.rows << ", width = " << ballot_box.cols << std::endl;
#endif
}

float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

int estimate_color(Image<HEIGHT, WIDTH, ap_uint<DATA_WIDTH_IN>> &candidate_regions_img, struct Circle &circle, cond_t cond_g, cond_t cond_y,
        cond_t cond_r) {
    int count_green = 0;
    int count_yellow = 0;
    int count_red = 0;
#if defined(DEBUG)
    std::cout << "Start estimate_color() " << std::endl;
    std::cout << "loop size: " << "height = " << candidate_regions_img.rows << ", width = "
            << candidate_regions_img.cols << std::endl;
#endif
    for (int i = 0; i < candidate_regions_img.rows; i++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=128
        for (int j = 0; j < candidate_regions_img.cols; j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=256
            if (distance((float) j, (float) i, circle.center_x, circle.center_y) < circle.radius) {
                ap_uint<32> pixel = candidate_regions_img.val[i][j];
                ap_uint8_t r = pixel.range(23, 16).to_uint();
                ap_uint8_t g = pixel.range(15, 8).to_uint();
                ap_uint8_t b = pixel.range(7, 0).to_uint();
                if (condition_green(r, g, b, cond_g)) {
                    count_green += 1;
                } else if (condition_yellow(r, g, b, cond_y)) {
                    count_yellow += 1;
                } else if (condition_red(r, g, b, cond_r)) {
                    count_red += 1;
                }
            }
        }
    }
#if defined(DEBUG)
    std::cout << "candidate circle : " << "r = " << circle.radius << "," << "center_y = " << circle.center_y << ", "
            << "center_x = " << circle.center_x << std::endl;
    std::cout << "green: " << count_green << ", yellow: " << count_yellow << ", red: " << count_red << std::endl;
#endif

    if (count_green > count_yellow && count_green > count_red) {
        return 3;
    } else if (count_yellow > count_green && count_yellow > count_red) {
        return 2;
    } else if (count_red > count_green && count_red > count_yellow) {
        return 1;
    }
    return 0;
}

void traffic_light_recognition(hls::stream<in_t> &stream_in, unsigned int rows, unsigned int cols, unsigned int radius,
        unsigned int cond_green[3], unsigned int cond_yellow[3], unsigned int cond_red[3], unsigned int *circle_center_x,
        unsigned int *circle_center_y, unsigned int *circle_radius, unsigned int *color) {
// input interface
#pragma HLS INTERFACE axis port=stream_in
#pragma HLS INTERFACE s_axilite port=rows
#pragma HLS INTERFACE s_axilite port=cols
#pragma HLS INTERFACE s_axilite port=radius
#pragma HLS INTERFACE s_axilite port=cond_green
#pragma HLS INTERFACE s_axilite port=cond_yellow
#pragma HLS INTERFACE s_axilite port=cond_red
// output interface
#pragma HLS INTERFACE s_axilite port=circle_center_x
#pragma HLS INTERFACE s_axilite port=circle_center_y
#pragma HLS INTERFACE s_axilite port=circle_radius
#pragma HLS INTERFACE s_axilite port=color
#pragma HLS INTERFACE s_axilite port=return

#pragma HLS ARRAY_PARTITION variable=sin_n complete dim=0
    Image<HEIGHT, WIDTH, ap_uint<DATA_WIDTH_IN>> candidate_regions_img(rows, cols);
#pragma HLS ARRAY_PARTITION variable=candidate_regions_img block factor=2
    struct Circle circle_ = { 0, 0, 0 };
    struct cond_t cond_g, cond_y, cond_r;
    cond_g.r = cond_green[0];
    cond_g.g = cond_green[1];
    cond_g.b = cond_green[2];
    cond_y.r = cond_yellow[0];
    cond_y.g = cond_yellow[1];
    cond_y.b = cond_yellow[2];
    cond_r.r = cond_red[0];
    cond_r.g = cond_red[1];
    cond_r.b = cond_red[2];
    extract_candidate_regions(stream_in, candidate_regions_img, cond_g, cond_y, cond_r);
    hough_circle(candidate_regions_img, radius, circle_);
    *color = estimate_color(candidate_regions_img, circle_, cond_g, cond_y, cond_r);
    *circle_center_x = circle_.center_x;
    *circle_center_y = circle_.center_y;
    *circle_radius = circle_.radius;
}
