#include "worker.h"
#include <iostream>
#include <filesystem>
#include <mutex>
#include <vector>
#include <algorithm>
#include <thread>

std::mutex db_mutex; /**< Mutex to ensure safe database access when using threads */
/**
 * @file worker.cpp
 * @brief Implementation of the Worker class in the EHS Management System.
 *
 * This file defines the behavior of the Worker class, including task reporting and
 * feedback submission functionalities. Workers can report task progress, submit
 * media files, and provide feedback on safety rules. Multithreading is used for 
 * concurrent task reporting, and database access is synchronized using mutexes.
 *
 * OOP Principles:
 * - Inheritance: Worker inherits from User.
 * - Encapsulation: Worker functionalities are organized in class methods.
 *
 * Multithreading:
 * - Task reporting is performed using threads to simulate concurrent workers.
 * - Mutex is used to synchronize database operations.
 * 
 */

/**
 * @brief Constructor for the Worker class.
 * 
 * Inherits from the User class to set the role as "worker."
 */
Worker::Worker() : User("worker") {}

/**
 * @brief Destructor for the Worker class.
 * 
 * Currently does nothing for cleanup.
 */
Worker::~Worker() {}

/**
 * @brief Worker reports progress on a task.
 * 
 * This method allows a worker to report their progress on a task. 
 * It uses threading to handle multiple task reports concurrently, 
 * allowing workers to submit reports simultaneously without waiting for each other.
 *
 * @param db Database connection.
 * @param userId Worker ID.
 *
 * OOP Principles:
 * The Worker class inherits from User and uses member functions to encapsulate 
 * behavior like reporting tasks and providing feedback.
 * 
 * Threading:
 * Threads are used to handle task reporting concurrently so multiple workers 
 * can submit reports at the same time.
 * 
 * Encapsulation:
 * Task reporting is encapsulated within the reportTaskWork function. 
 * The database interactions are isolated inside the method, making it easy to manage 
 * the worker's task reporting logic separately.
 */
void Worker::reportTaskWork(sqlite3* db, int userId) {
    int taskId;
    std::string reportDesc, mediaPath;
    std::vector<int> validTaskIds;

    // Fetch assigned tasks
    const char* query = "SELECT id, task_description, status FROM tasks WHERE worker_id = ? AND status != 'completed';";
    sqlite3_stmt* stmtList;

    {
        std::lock_guard<std::mutex> lock(db_mutex);
        if (sqlite3_prepare_v2(db, query, -1, &stmtList, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmtList, 1, userId);
            int count = 1;

            std::cout << "\nAssigned Tasks:\n";
            while (sqlite3_step(stmtList) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmtList, 0);
                std::string desc = reinterpret_cast<const char*>(sqlite3_column_text(stmtList, 1));
                std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmtList, 2));

                std::cout << count++ << ". Task ID: " << id << " | Description: " << desc << " | Status: " << status << "\n";
                validTaskIds.push_back(id);
            }
            sqlite3_finalize(stmtList);
        } else {
            std::cerr << "Failed to fetch assigned tasks.\n";
            return;
        }
    }

    // Let worker select a task
    while (true) {
        std::cout << "\nEnter Task ID to report: ";
        std::cin >> taskId;

        if (std::cin.fail() || taskId < 0 || std::find(validTaskIds.begin(), validTaskIds.end(), taskId) == validTaskIds.end()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid or unassigned task ID. Try again.\n";
            continue;
        }

        std::cin.ignore();  // Clear newline character
        break;
    }

    std::cout << "Enter report description: ";
    std::getline(std::cin, reportDesc);

    std::cout << "Enter path to media file: ";
    std::getline(std::cin, mediaPath);

    // Threaded task report function
    auto reportTask = [=]() {
        auto start = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "\n[Thread] Worker " << userId << " reporting Task " << taskId
                  << " | Start: " << std::ctime(&start);

        // Simulate a long-running task (e.g., file upload)
        std::this_thread::sleep_for(std::chrono::seconds(180));

        std::string savedFilePath = "./uploads/task_" + std::to_string(taskId) + "_user_" + std::to_string(userId);
        std::filesystem::create_directories("./uploads");

        try {
            std::filesystem::copy(mediaPath, savedFilePath, std::filesystem::copy_options::overwrite_existing);
        } catch (const std::exception& e) {
            std::cerr << "Failed to save media: " << e.what() << "\n";
            return;
        }

        // Update task details in database
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "UPDATE tasks SET worker_report = ?, worker_media = ?, status = 'completed' WHERE id = ? AND worker_id = ?;";

        std::lock_guard<std::mutex> lock(db_mutex);
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, reportDesc.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, savedFilePath.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, taskId);
            sqlite3_bind_int(stmt, 4, userId);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                std::cout << "Task report submitted successfully.\n";
            } else {
                std::cerr << "Failed to submit report.\n";
            }

            sqlite3_finalize(stmt);
        } else {
            std::cerr << "Failed to prepare statement.\n";
        }

        auto end = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "[Thread] Worker " << userId << " finished Task " << taskId
                  << " | End: " << std::ctime(&end);
    };

    // Start the task report thread
    std::thread reportThread(reportTask);
    reportThread.join();  // You can remove `.join()` to let threads run in background
}

/**
 * @brief Worker provides feedback on a rule.
 * 
 * Workers can select a rule and submit feedback for it.
 *
 * @param db Database connection.
 *
 * OOP Principles:
 * The feedback functionality is encapsulated in the `GiveRuleFeedback` method, which 
 * ensures that the worker's feedback is handled properly and isolated within the method.
 * 
 * Encapsulation:
 * Feedback collection and saving are encapsulated inside the method, providing 
 * a clean interface for workers to give feedback on rules.
 */
void Worker::GiveRuleFeedback(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;

    // Show available rules
    std::cout << "\n--- Available Rules ---\n";
    const char* listRulesSql = "SELECT id, rule_text FROM rules;";
    if (sqlite3_prepare_v2(db, listRulesSql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::cout << "Rule ID: " << id << " | " << text << "\n";
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to load rules.\n";
        return;
    }

    // Let worker provide feedback
    int ruleId;
    std::string feedback;

    while (true) {
        std::cout << "\nEnter Rule ID to give feedback: ";
        std::cin >> ruleId;

        if (std::cin.fail() || ruleId < 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid Rule ID. Try again.\n";
        } else {
            break;
        }
    }

    std::cin.ignore();

    std::cout << "Enter your feedback: ";
    std::getline(std::cin, feedback);

    // Save feedback in the database
    const char* updateSql = "UPDATE rules SET feedback = ? WHERE id = ?";
    if (sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, feedback.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, ruleId);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            std::cout << "Feedback submitted successfully.\n";
        } else {
            std::cout << "Failed to submit feedback.\n";
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare statement.\n";
    }
}
