#include <iostream>
#include "db/Database.h"
#include "manager/manager.h"
#include "worker/worker.h"
#include "user/user.h"
#include <unistd.h>
#include <fstream>

/**
 * @mainpage Environment, Health, and Safety (EHS) Management System
 *
 * @section intro_sec Introduction
 * This is a C++ terminal-based EHS management system for handling workers and managers.
 * It supports user registration, login, task assignment, violation reporting,
 * safety rules, feedback collection, using SQLite.
 *
 * @section features_sec Features
 * - Manager and Worker roles
 * - Task assignment and reporting
 * - Violation reporting
 * - Rule addition, viewing, and feedback
 * - Multithreading support
 *
 * @section structure_sec Folder Structure
 * - `db/`: Database connection and setup
 * - `manager/`: Manager class and functions
 * - `worker/`: Worker class and functions
 * - `user/`: Base User class
 *
 * @section howto_sec How to Compile
 * Compile using:
 * @code
 * g++ code.cpp db/Database.cpp manager/manager.cpp user/user.cpp worker/worker.cpp -lsqlite3 -lssl -lcrypto
 * @endcode
 *
 * @section usage_sec Usage
 * Run the app and follow the terminal prompts to register/login and perform role-based actions.
 */


void handleWorkerMenu(sqlite3* db, const std::string& username, const std::string& password) {
    Worker w;
    int choice;
    int userId;
    userId = w.getUserId(db, username, password);

    do {
        std::cout << "\n--- Worker Menu ---\n";
        std::cout << "1. View Assigned Tasks\n";
        std::cout << "2. Report Task Work\n";
        std::cout << "3. View Safety Rules\n";
        std::cout << "4. Give Feedback for Rules\n";
        std::cout << "5. View Feedback of Rules\n";
        std::cout << "0. Logout\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                w.viewTaskDetails(db, userId);
                break;
            case 2:
                w.reportTaskWork(db, userId);
                break;
            case 3:
                w.viewRules(db);
                break;
            case 4:
                w.GiveRuleFeedback(db);
                break;
            case 5:
                w.ViewRuleFeedback(db);
                break;
            case 0:
                std::cout << "Logging out...\n";
                break;
            default:
                std::cout << "Invalid choice!\n";
        }
    } while (choice != 0);
}

void handleManagerMenu(sqlite3* db) {
    Manager m;
    int choice;

    do {
        std::cout << "\n--- Manager Menu ---\n";
        std::cout << "1. Assign Task\n";
        std::cout << "2. Report Violation\n";
        std::cout << "3. View Rules\n";
        std::cout << "4. Add Rule\n";
        std::cout << "5. View Feedback of rules\n";
        std::cout << "6. View Assigned Tasks\n";
        std::cout << "7. Delete the task\n";
        std::cout << "8. Delete rules\n";
        std::cout << "0. Logout\n";
        std::cout << "Enter choice: ";
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                m.assignTask(db);
                break;
            case 2:
                m.reportViolation(db);
                break;
            case 3:
                m.viewRules(db);
                break;
            case 4:
                m.addRule(db);
                break;
            case 5:
                m.ViewRuleFeedback(db);
                break;
            case 6:
                m.viewTaskDetails(db, 0, true);
                break;
            case 7:
                m.deleteTask(db);
                break;
            case 8:
                m.deleteRule(db);
                break;
            case 0:
                std::cout << "Logging out...\n";
                break;
            default:
                std::cout << "Invalid choice!\n";
        }
    } while (choice != 0);
}

void handleRegistration(sqlite3* db, const std::string& username, const std::string& password) {
    std::string role;
    while (true) {
        std::cout << "Enter role (worker/manager): ";
        std::getline(std::cin, role);
        if (role.empty()) {
            std::cout << "Role cannot be empty. Please enter a valid role.\n";
        } else if (role != "worker" && role != "manager") {
            std::cout << "Invalid role. Please enter 'worker' or 'manager'.\n";
        } else {
            break;
        }
    }

    if (role == "worker") {
        Worker w;
        if (w.userExists(db, username, password))
            std::cout << "User already exists with these credentials!\n";
        else if (w.registerUser(db, username, password))
            std::cout << "Registration Successfull!\n";
        else
            std::cout << "Registration failed!\n";

    } else if (role == "manager") {
        Manager m;
        if (m.userExists(db, username, password))
            std::cout << "User already exists with these credentials!\n";
        else if (m.registerUser(db, username, password))
            std::cout << "Registration Successfull!\n";
        else
            std::cout << "Registration failed!\n";

    } else {
        std::cout << "Invalid role entered!\n";
    }
}

void handleLogin(sqlite3* db, const std::string& username, const std::string& password) {
    User u;
    std::string role = u.getUserRole(db, username, password);
    if (role == "worker") {
        Worker w;
        if (w.loginUser(db, username, password)) {
            std::cout << "Logged in successfully!\n";
            handleWorkerMenu(db, username, password);
        } else {
            std::cout << "Login failed!\n";
        }
    } else if (role == "manager") {
        Manager m;
        if (m.loginUser(db, username, password)) {
            std::cout << "Logged in successfully!\n";
            handleManagerMenu(db);
        } else {
            std::cout << "Login failed!\n";
        }
    } else {
        std::cout << "Invalid credentials!\n";
    }
}

int main() {
    DatabaseManager dbManager("ehs.db");
    dbManager.setupTables();
    sqlite3* db = dbManager.getDB();

    int choice;

    while (true) {
        std::cout << "\n=== EHS System ===\n";
        std::cout << "1. Register\n2. Login\n0. Exit\nChoice: ";
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 0) {
            std::cout << "Exiting...\n";
            break;
        }

        std::string username, password;

        while (true) {
            std::cout << "Username: ";
            std::getline(std::cin, username);
            if (username.empty()) {
                std::cout << "Username cannot be empty. Please enter a valid username.\n";
            } else {
                break; // Exit the loop if the username is valid
            }
        }
        
        while (true) {
            std::cout << "Password: ";
            std::getline(std::cin, password);
            if (password.empty()) {
                std::cout << "Password cannot be empty. Please enter a valid password.\n";
            } else {
                break; // Exit the loop if the password is valid
            }
        }
        switch (choice) {
            case 1:
                handleRegistration(db, username, password);
                break;
            case 2:
                handleLogin(db, username, password);
                break;
            default:
                std::cout << "Invalid choice!\n";
                break;
        }
    }

    return 0;
}
