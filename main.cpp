#include "dehaze.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#include <conio.h> // For _getch() on Windows
#endif

// Simple function to check if a file exists
bool fileExists(const std::string& filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}

// Function to wait for a key press without blocking the return to menu
void waitForKeyPress() {
    std::cout << "\nPress any key to return to menu..." << std::endl;

#ifdef _WIN32
    _getch(); // Use _getch() on Windows which doesn't require Enter key
#else
    // For non-Windows platforms
    getchar();
#endif
}

// Function to show file open dialog (ANSI version)
std::string getImageFile() {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Image Files (*.jpg;*.jpeg;*.png;*.bmp)\0*.jpg;*.jpeg;*.png;*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "jpg";

    if (GetOpenFileNameA(&ofn)) {
        return std::string(fileName);
    }
    return "";
#else
    // For non-Windows platforms, use simple input
    std::string filePath;
    std::cout << "Enter path to image file: ";
    std::getline(std::cin, filePath);
    return filePath;
#endif
}

// Function to process a single image
bool processImage(const std::string& img_path, const std::string& project_path) {
    // Use forward slashes consistently
    std::string normalized_path = img_path;
    std::replace(normalized_path.begin(), normalized_path.end(), '\\', '/');

    std::cout << "Processing image: " << normalized_path << std::endl;

    // Load image
    cv::Mat img = cv::imread(normalized_path);
    if (img.empty()) {
        std::cerr << "Error: Failed to load image: " << normalized_path << std::endl;
        waitForKeyPress();
        return false;
    }

    // Extract just the filename for the output file
    size_t pos = normalized_path.find_last_of('/');
    std::string img_name = (pos != std::string::npos) ? normalized_path.substr(pos + 1) : normalized_path;

    std::cout << "Image loaded successfully. Size: " << img.cols << "x" << img.rows << std::endl;

    // Start measuring total execution time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Process image
    cv::Mat res = DarkChannel::dehaze(img);

    // End measuring total execution time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    std::cout << "Dehazing completed. Result size: " << res.rows << "x" << res.cols << std::endl;
    std::cout << "Total execution time: " << std::fixed << std::setprecision(10) << duration << " ms" << std::endl;

    // Get timing info if available in your implementation
    // If you have the TimingInfo class and getLastTimingInfo() function, uncomment these lines
    /*
    DarkChannel::TimingInfo timing = DarkChannel::getLastTimingInfo();
    std::cout << "\n===== PERFORMANCE BREAKDOWN =====" << std::endl;
    std::cout << "Dark Channel Calculation: " << timing.darkChannelTime << " ms" << std::endl;
    std::cout << "Atmospheric Light Estimation: " << timing.atmosphericLightTime << " ms" << std::endl;
    std::cout << "Transmission Estimation: " << timing.transmissionTime << " ms" << std::endl;
    std::cout << "Transmission Refinement: " << timing.refinementTime << " ms" << std::endl;
    std::cout << "Scene Reconstruction: " << timing.reconstructionTime << " ms" << std::endl;
    std::cout << "Total Execution Time: " << timing.totalTime << " ms" << std::endl;
    std::cout << "================================" << std::endl;
    */

    // Save in the project directory
    std::string output_path = project_path + "dehazed_" + img_name;
    cv::imwrite(output_path, res);
    std::cout << "Result saved as: " << output_path << std::endl;

    // Create named windows with the WINDOW_NORMAL flag to allow resizing
    cv::namedWindow("Original", cv::WINDOW_NORMAL);
    cv::namedWindow("Dehazed Result", cv::WINDOW_NORMAL);

    // Position the windows side by side
    cv::moveWindow("Original", 50, 50);  // Position original image window
    cv::moveWindow("Dehazed Result", 50 + img.cols + 20, 50);  // Position result window to the right

    // Display the result
    cv::imshow("Original", img);
    cv::imshow("Dehazed Result", res);

    // First wait for key in OpenCV window (this won't block the return to menu)
    std::cout << "\nViewing results. Press any key in the image window to continue..." << std::endl;
    cv::waitKey(0); // wait for a key press in the OpenCV window

    // Close the windows to prevent stacking up multiple windows
    cv::destroyWindow("Original");
    cv::destroyWindow("Dehazed Result");
    cv::destroyAllWindows(); // Make sure all windows are closed

    // Then wait for key in console to ensure we return to menu
    waitForKeyPress();

    return true;
}

int main(int argc, char** argv) {
    try {
        std::string project_path = "C:/Users/esther/source/repos/Dehaze/";

        // Check if we should use interactive mode
        bool interactive_mode = true;
        std::string img_path;

        if (argc > 1) {
            // If arguments provided, process the specified image
            interactive_mode = false;
            std::string arg_path = argv[1];

            // Check if the argument is a relative path
            if (arg_path.find(':') == std::string::npos) {
                // It's a relative path, combine with project path
                img_path = project_path + arg_path;
            }
            else {
                // It's an absolute path, use it directly
                img_path = arg_path;
            }

            processImage(img_path, project_path);

            // After processing command line image, enter interactive mode
            interactive_mode = true;
        }

        // Interactive mode loop
        if (interactive_mode) {
            int choice = -1;
            while (choice != 0) {
                std::cout << "\n=== Dehaze Interactive Mode ===" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  1: Enter image path manually" << std::endl;
                std::cout << "  2: Use file dialog to select image" << std::endl;
                std::cout << "  3: Process multiple images" << std::endl;
                std::cout << "  0: Exit" << std::endl;

                std::cout << "\nEnter your choice (0-3): ";
                std::cin >> choice;
                std::cin.ignore(); // Clear the newline

                switch (choice) {
                case 0:
                    std::cout << "Exiting program." << std::endl;
                    break;

                case 1: {
                    // Manual path entry
                    std::cout << "Enter image path (or 'exit' to return to menu): ";
                    std::getline(std::cin, img_path);
                    if (img_path == "exit" || img_path == "quit") {
                        break;
                    }
                    processImage(img_path, project_path);
                    break;
                }

                case 2: {
                    // File dialog
                    img_path = getImageFile();
                    if (!img_path.empty()) {
                        processImage(img_path, project_path);
                    }
                    else {
                        std::cout << "No file selected." << std::endl;
                        waitForKeyPress();
                    }
                    break;
                }

                case 3: {
                    // Process multiple images
                    std::cout << "Enter 'exit' at any time to return to menu." << std::endl;
                    while (true) {
                        std::cout << "\nSelect option for next image:" << std::endl;
                        std::cout << "  1: Enter path manually" << std::endl;
                        std::cout << "  2: Use file dialog" << std::endl;
                        std::cout << "  0: Return to main menu" << std::endl;

                        int subChoice;
                        std::cout << "Choice: ";
                        std::cin >> subChoice;
                        std::cin.ignore(); // Clear the newline

                        if (subChoice == 0) {
                            break;
                        }
                        else if (subChoice == 1) {
                            std::cout << "Enter image path: ";
                            std::getline(std::cin, img_path);
                            if (img_path == "exit" || img_path == "quit") {
                                break;
                            }
                            processImage(img_path, project_path);
                        }
                        else if (subChoice == 2) {
                            img_path = getImageFile();
                            if (img_path.empty()) {
                                std::cout << "No file selected." << std::endl;
                                waitForKeyPress();
                                continue;
                            }
                            processImage(img_path, project_path);
                        }
                        else {
                            std::cout << "Invalid choice." << std::endl;
                            waitForKeyPress();
                            continue;
                        }
                    }
                    break;
                }

                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
                    waitForKeyPress();
                }
            }
        }

        cv::destroyAllWindows();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        waitForKeyPress();
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        waitForKeyPress();
        return 1;
    }
}