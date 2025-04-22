#include "manager.h"
#include <iostream>
#include <ctime>

Manager::Manager() : User("manager") {}
Manager::~Manager() {}
/**
 * @file manager.cpp
 * @brief Implements the Manager class functionalities for the EHS Management System.
 *
 * This file defines the behavior of the Manager role, including:
 * - Assigning tasks to workers.
 * - Reporting violations related to tasks.
 * - Adding and deleting safety rules.
 * - Deleting existing tasks.
 *
 * The class uses SQLite for all database operations and follows object-oriented
 * principles. It is intended to be used in conjunction with the overall EHS 
 * system that manages Environment, Health, and Safety processes.
 *
 */

/**
 * @brief Assigns a task to a worker.
 *
 * Displays a list of available workers, prompts the manager
 * for task details, and inserts the task into the database.
 *
 * @param db Pointer to the SQLite database connection.
 */
void Manager::assignTask(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    int workerId;
    std::string task, username;

    // Show list of workers
    std::cout << "\n--- Available Workers ---\n";
    const char* queryWorkers = "SELECT id, username FROM users WHERE role = 'worker';";
    if (sqlite3_prepare_v2(db, queryWorkers, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* uname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::cout << "ID: " << id << " | Username: " << uname << "\n";
        }
    }
    sqlite3_finalize(stmt);
    std::cout << "\n";

    // Input validation for worker ID
    while (true) {
        std::cout << "Enter worker ID (non-negative number): ";
        std::cin >> workerId;
        std::cout << "\n";
    
        if (std::cin.fail() || workerId < 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please try again.\n";
            std::cout << "\n";
        } else {
            std::cin.ignore();
            break;
        }
    }

    // Task description input
    std::cout << "Enter task description: ";
    std::getline(std::cin, task);

    // Fetch username for the worker
    const char* getUserSql = "SELECT username FROM users WHERE id = ?;";
    if (sqlite3_prepare_v2(db, getUserSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, workerId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        } else {
            std::cout << "Worker not found.\n";
            sqlite3_finalize(stmt);
            return;
        }
    }
    sqlite3_finalize(stmt);

    // Insert task into the database
    const char* insertSql = "INSERT INTO tasks (worker_id, worker_username, task_description, status) VALUES (?, ?, ?, 'pending');";
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, workerId);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, task.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            std::cout << "Task assigned successfully.\n";
        } else {
            std::cout << "Failed to assign task.\n";
        }
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Reports a violation associated with a task.
 *
 * Lists all pending tasks, allows the manager to select a task,
 * and records the violation details along with the updated task status.
 *
 * @param db Pointer to the SQLite database connection.
 */
void Manager::reportViolation(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    int taskId;
    std::string comment, status;

    // Display all pending tasks
    std::cout << "\n--- Assigned Tasks ---\n";
    const char* getTasks = "SELECT id, worker_username, task_description FROM tasks WHERE status = 'pending';";
    if (sqlite3_prepare_v2(db, getTasks, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* user = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::cout << "Task ID: " << id << " | Assigned To: " << user << "\nDescription: " << desc << "\n------------------------\n";
        }
    }
    sqlite3_finalize(stmt);

    // Input validation for task ID
    while (true) {
        std::cout << "\nEnter Task ID to report violation: ";
        std::cin >> taskId;
    
        if (std::cin.fail() || taskId < 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Task ID must be a non-negative number.\n";
        } else {
            std::cin.ignore();
            break;
        }
    }
    
    // Input validation for task status
    while (true) {
        std::cout << "Enter new task status (e.g., violation, incomplete): ";
        std::getline(std::cin, status);

        // Check if the input is purely numeric
        bool isNumeric = true;
        for (char c : status) {
            if (!isdigit(c)) {
                isNumeric = false;
                break;
            }
        }
    
        if (status.empty() || isNumeric) {
            std::cout << "Invalid input. Task status must be a non-numeric string.\n";
        } else {
            break;
        }
    }

    // Get current timestamp
    time_t now = time(0);
    std::string timestamp = ctime(&now);
    if (!timestamp.empty() && timestamp.back() == '\n') {
        timestamp.pop_back();
    }

    // Update task with violation details
    const char* updateSql = "UPDATE tasks SET status = ?, violation_comment = ?, violation_timestamp = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, comment.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, timestamp.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, taskId);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            std::cout << "Task updated with violation info.\n";
        } else {
            std::cout << "Failed to update task.\n";
        }
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Adds a new safety rule to the system.
 *
 * Prompts the manager for a safety rule text and inserts it into
 * the rules table in the database with the current timestamp.
 *
 * @param db Pointer to the SQLite database connection.
 */
void Manager::addRule(sqlite3* db) {
    std::string rule;
    while (true) {
        std::cout << "Enter the new safety rule: ";
        std::getline(std::cin, rule);
        
        if (rule.empty()) {
            std::cout << "Rule cannot be empty. Please enter a valid safety rule.\n";
        } else {
            break; 
        }
    }

    // Get current timestamp
    std::time_t now = std::time(nullptr);
    char timestamp[100];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // Insert rule into the database
    std::string query = "INSERT INTO rules (rule_text, timestamp) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, rule.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, timestamp, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "New rule added successfully.\n";
    }

    sqlite3_finalize(stmt);
}

/**
 * @brief Deletes an existing safety rule from the system.
 *
 * Displays all rules and allows the manager to select a rule ID
 * to delete from the database.
 *
 * @param db Pointer to the SQLite database connection.
 */
void Manager::deleteRule(sqlite3* db) {
    // Display all rules
    const char* listSql = "SELECT id, rule_text FROM rules;";
    sqlite3_stmt* listStmt;

    if (sqlite3_prepare_v2(db, listSql, -1, &listStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to retrieve rules: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "\n--- Existing Rules ---\n";
    while (sqlite3_step(listStmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(listStmt, 0);
        const unsigned char* ruleText = sqlite3_column_text(listStmt, 1);
        std::cout << "ID: " << id << " | Rule: " << ruleText << '\n';
    }
    sqlite3_finalize(listStmt);

    // Input validation for rule ID
    int ruleId;
    while (true) {
        std::cout << "\nEnter Rule ID to delete: ";
        std::cin >> ruleId;
        if (std::cin.fail() || ruleId < 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Rule ID must be a non-negative number.\n";
        } else {
            break;
        }
    }
    std::cin.ignore();

    // Delete rule from database
    const char* deleteSql = "DELETE FROM rules WHERE id = ?;";
    sqlite3_stmt* delStmt;

    if (sqlite3_prepare_v2(db, deleteSql, -1, &delStmt, nullptr) != SQLITE_OK) {
        std::cout << "Couldn't prepare the delete statement.\n";
        return;
    }

    sqlite3_bind_int(delStmt, 1, ruleId);

    if (sqlite3_step(delStmt) == SQLITE_DONE) {
        std::cout << "Rule deleted successfully.\n";
    } else {
        std::cout << "Failed to delete rule.\n";
    }

    sqlite3_finalize(delStmt);
}

/**
 * @brief Deletes an existing task from the system.
 *
 * Displays all tasks and allows the manager to select a task ID
 * to delete from the database.
 *
 * @param db Pointer to the SQLite database connection.
 */
void Manager::deleteTask(sqlite3* db) {
    // Display all tasks
    const char* listSql = "SELECT id, task_description, worker_username FROM tasks;";
    sqlite3_stmt* listStmt;

    if (sqlite3_prepare_v2(db, listSql, -1, &listStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to retrieve tasks: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "\n--- Existing Tasks ---\n";
    while (sqlite3_step(listStmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(listStmt, 0);
        const unsigned char* taskDescription = sqlite3_column_text(listStmt, 1);
        const unsigned char* workerUsername = sqlite3_column_text(listStmt, 2);
        std::cout << "ID: " << id << " | Worker: " << workerUsername << " | Task: " << taskDescription << '\n';
    }
    sqlite3_finalize(listStmt);

    // Input validation for task ID
    int taskId;
    while (true) {
        std::cout << "\nEnter Task ID to delete: ";
        std::cin >> taskId;
        if (std::cin.fail() || taskId < 0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Task ID must be a non-negative number.\n";
        } else {
            break;
        }
    }
    std::cin.ignore();

    // Delete task from database
    const char* deleteSql = "DELETE FROM tasks WHERE id = ?;";
    sqlite3_stmt* delStmt;

    if (sqlite3_prepare_v2(db, deleteSql, -1, &delStmt, nullptr) != SQLITE_OK) {
        std::cout << "Couldn't prepare the delete statement.\n";
        return;
    }

    sqlite3_bind_int(delStmt, 1, taskId);

    if (sqlite3_step(delStmt) == SQLITE_DONE) {
        std::cout << "Task deleted successfully.\n";
    } else {
        std::cout << "Failed to delete task.\n";
    }

    sqlite3_finalize(delStmt);
}
