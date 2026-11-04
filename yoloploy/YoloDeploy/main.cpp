#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

// ================= Config =================
const string ONNX_PATH = "C:/Users/Lenovo/Desktop/shentong/runs/detect/train-3/weights/best.onnx";
const float CONF_THRESHOLD = 0.25f;
const float NMS_THRESHOLD = 0.45f;
const int INPUT_SIZE = 640;
vector<string> CLASS_NAMES = { "bottle", "soda_can", "plastic", "shell" };

int main() {
    // 1. Load Model
    Net net = readNetFromONNX(ONNX_PATH);
    net.setPreferableBackend(DNN_BACKEND_OPENCV);
    net.setPreferableTarget(DNN_TARGET_CPU);
    cout << "Model Loaded!" << endl;

    // 2. Open Camera (0 is usually the default webcam)
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Cannot open camera!" << endl;
        return -1;
    }

    // Set camera resolution (optional, can improve speed)
    cap.set(CAP_PROP_FRAME_WIDTH, 640);
    cap.set(CAP_PROP_FRAME_HEIGHT, 480);

    Mat img;
    int frame_count = 0;
    double total_time = 0;

    cout << "Press 'q' or 'ESC' to quit." << endl;

    while (true) {
        cap >> img;
        if (img.empty()) break;

        int img_w = img.cols;
        int img_h = img.rows;

        // Preprocess
        Mat blob;
        blobFromImage(img, blob, 1.0 / 255.0, Size(INPUT_SIZE, INPUT_SIZE), Scalar(), true, false);

        // Inference and Timing
        net.setInput(blob);
        double t = (double)getTickCount();
        Mat output = net.forward();
        t = ((double)getTickCount() - t) / getTickFrequency();

        double time_ms = t * 1000;
        total_time += time_ms;
        frame_count++;

        // Postprocess
        Mat output_trans;
        transpose(output.reshape(1, 8), output_trans);

        vector<Rect> boxes;
        vector<float> confidences;
        vector<int> class_ids;

        for (int i = 0; i < output_trans.rows; ++i) {
            Mat row = output_trans.row(i);
            float cx = row.at<float>(0, 0);
            float cy = row.at<float>(0, 1);
            float w = row.at<float>(0, 2);
            float h = row.at<float>(0, 3);

            float max_conf = 0;
            int max_id = -1;
            for (int c = 0; c < CLASS_NAMES.size(); ++c) {
                float conf = row.at<float>(0, 4 + c);
                if (conf > max_conf) {
                    max_conf = conf;
                    max_id = c;
                }
            }

            if (max_conf > CONF_THRESHOLD) {
                // Scale back to original image size
                int left = int((cx - w / 2.0) * (float)img_w / INPUT_SIZE);
                int top = int((cy - h / 2.0) * (float)img_h / INPUT_SIZE);
                int width = int(w * (float)img_w / INPUT_SIZE);
                int height = int(h * (float)img_h / INPUT_SIZE);

                boxes.push_back(Rect(left, top, width, height));
                confidences.push_back(max_conf);
                class_ids.push_back(max_id);
            }
        }

        // NMS
        vector<int> indices;
        NMSBoxes(boxes, confidences, CONF_THRESHOLD, NMS_THRESHOLD, indices);

        // Draw Results
        for (int idx : indices) {
            Rect box = boxes[idx];
            int class_id = class_ids[idx];
            float conf = confidences[idx];

            rectangle(img, box, Scalar(0, 255, 0), 2);
            string label = CLASS_NAMES[class_id] + ": " + to_string(conf).substr(0, 4);
            putText(img, label, Point(box.x, box.y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 2);
        }

        // Show FPS and Time on screen
        double avg_time = total_time / frame_count;
        double fps = 1000.0 / avg_time;
        string info = "Time: " + to_string(time_ms).substr(0, 4) + " ms | FPS: " + to_string(fps).substr(0, 4);
        putText(img, info, Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 0, 0), 2);

        imshow("C++ Real-time Inference", img);

        // Press 'q' or ESC to quit
        char c = (char)waitKey(1);
        if (c == 27 || c == 'q' || c == 'Q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}