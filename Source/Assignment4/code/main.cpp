#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4)
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window)
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t)
{
    // TODO: Implement de Casteljau's algorithm
    std::vector<cv::Point2f> ctl_points = control_points;
    std::vector<cv::Point2f> tmp_points;
    while(tmp_points.size() != 1){
        tmp_points.clear();
        for(int i = 0; i < ctl_points.size() - 1 ; ++i)
        {
            tmp_points.push_back((1-t)* ctl_points[i] + t * ctl_points[i+1]);
        }
        ctl_points = tmp_points;
    }
    return ctl_points[0];
}

void color_and_anti_alias(const cv::Point2f& point, cv::Mat& window, std::function<void(float, float, float)> color_callback)
{
    cv::Point2f center = {(int)(point.x) * 1.f, (int)(point.y)*1.f};
    auto calc_color = [](float x, float y, cv::Point2f point)->float
    {
        float dis = sqrt((point.x - x) * (point.x - x) + (point.y - y) * (point.y - y));
        return std::max(0.0f, 255.f * (-1.f / (2 * 1.41f) * dis  + 1.f));
        //return std::max(0.0f, 255 * (-0.125f * dis * dis  + 1));
    };
    // left up
    color_callback(center.x - 1, center.y + 1, calc_color(center.x - 1, center.y+1,  point));
    // left center
    color_callback(center.x - 1, center.y, calc_color(center.x - 1, center.y,  point));
    // left down
    color_callback(center.x - 1, center.y-1, calc_color(center.x - 1, center.y - 1,  point));
    // center up
    color_callback(center.x, center.y+1, calc_color(center.x, center.y+1,  point));
    //center
    color_callback(center.x, center.y, calc_color(center.x, center.y,  point));
    // center down
    color_callback(center.x, center.y-1, calc_color(center.x, center.y-1,  point));
    // right up
    color_callback(center.x + 1, center.y + 1, calc_color(center.x + 1, center.y + 1,  point));
    // right center
    color_callback(center.x + 1, center.y, calc_color(center.x + 1, center.y,  point));
    // right down
    color_callback(center.x + 1, center.y - 1, calc_color(center.x + 1, center.y - 1,  point));

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.

    auto add_color = [&](float x, float y, float c)
    {
        if(x < 0 || y < 0)
            return;
        window.at<cv::Vec3b>(y, x)[1] = c;
    };

    const float steps = 1000;
    for(int i = 0; i < steps; ++i)
    {
        auto point = recursive_bezier(control_points, 1.f * i / steps);
        color_and_anti_alias(point, window, add_color);
        // window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    }
}

int main()
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27)
    {
        for (auto &point : control_points)
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4)
        {
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
