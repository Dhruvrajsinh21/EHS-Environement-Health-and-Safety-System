#include "user.h"
#include <iostream>
#include <openssl/sha.h>
#include <string>
#include <iomanip>
/**
 * @file user.cpp
 * @brief Implementation of the User class for login and registration in the EHS Management System.
 *
 * This file contains the definitions for the User class methods, including user registration,
 * login, and helper functions to interact with the SQLite database. It uses object-oriented
 * principles such as encapsulation and abstraction.
 *
 */

/// @brief Default constructor for User class (used in OOP).
User::User() {}

/// @brief Parameterized constructor that sets the user role (OOP: Encapsulation).
/// @param role The role to assign (e.g., "worker", "manager")
User::User(const std::string& role) {
    this->role = role;
}

/// @brief Destructor for the User class (OOP: Clean-up if needed).
User::~User() {}

/// @brief Register a new user with hashed password (OOP: Behavior using class method).
/// @param db SQLite database pointer.
/// @param username The user's name.
/// @param password The user's password (plain text).
/// @return True if registration is successful.
bool User::registerUser(sqlite3* db, const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Username and password cannot be empty.\n";
        return false;
    }

    std::string hashed = hashPassword(password);
    const char* sql = "INSERT INTO users (username, password, role) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

/// @brief Log in user by checking hashed password and role.
/// @param db SQLite database pointer.
/// @param username Username entered.
/// @param password Password entered.
/// @return True if login is successful.
bool User::loginUser(sqlite3* db, const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Username and password cannot be empty.\n";
        return false;
    }

    std::string hashed = hashPassword(password);
    const char* sql = "SELECT * FROM users WHERE username=? AND password=? AND role=?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return success;
}

/// @brief Hash a password using SHA-256 (OOP: Abstraction, hides hashing logic).
/// @param password Plain text password.
/// @return Hashed password in hex string.
string User::hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);
    stringstream ss;
    for (unsigned char c : hash)
        ss << hex << setw(2) << setfill('0') << (int)c;
    return ss.str();
}

/// @brief Get the role of a user after checking login credentials.
/// @param db SQLite database pointer.
/// @param username User’s name.
/// @param password User’s password.
/// @return Role if found; otherwise "none".
std::string User::getUserRole(sqlite3* db, const std::string& username, const std::string& password) {
    std::string role = "none";
    string hashed = hashPassword(password);
    const char* sql = "SELECT role FROM users WHERE username = ? AND password = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_TRANSIENT);

        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_ROW) {
            const unsigned char* roleText = sqlite3_column_text(stmt, 0);
            if (roleText) {
                role = reinterpret_cast<const char*>(roleText);
            }
        }
    } else {
        std::cerr << "SQL error in getUserRole: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return role;
}

/// @brief Check if user already exists (by username + password).
/// @param db SQLite database.
/// @param username Username.
/// @param password Password.
/// @return True if user exists.
bool User::userExists(sqlite3* db, const std::string& username, const std::string& password) {
    sqlite3_stmt* stmt;
    string hashed = hashPassword(password);
    const char* sql = "SELECT id FROM users WHERE username = ? AND password = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    bool exists = (result == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return exists;
}

/// @brief View task details. Manager sees all, worker sees their own tasks.
/// @param db SQLite DB.
/// @param userId User's ID.
/// @param isManager If true, show all tasks.
void User::viewTaskDetails(sqlite3* db, int userId, bool isManager) {
    const char* sql = isManager 
        ? "SELECT id, worker_id, worker_username, task_description, status, violation_comment, violation_timestamp, worker_report, worker_media FROM tasks;"
        : "SELECT id, worker_username, task_description, status, violation_comment, violation_timestamp, worker_report, worker_media FROM tasks WHERE worker_id = ?;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    if (!isManager) {
        sqlite3_bind_int(stmt, 1, userId);
    }

    std::cout << "\n=== Task Details ===\n";
    int taskNumber = 1;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);

        std::cout << taskNumber << ".\n";

        if (isManager) {
            int assignedTo = sqlite3_column_int(stmt, 1);
            std::string worker_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            const char* violationcomment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const char* violationtimestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            const char* worker_report = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            const char* worker_media = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

            std::cout << "Task ID: " << id << "\n\n" << "Assigned To: " << worker_username << "\n\n"
                      << "Task given: " << desc << "\n\n"
                      << "Status: " << status << "\n\n"
                      << "Violation Comment: " << (violationcomment ? violationcomment : "None") << "\n\n"
                      << "Violation Timestamp: " << (violationtimestamp ? violationtimestamp : "None") << "\n\n"
                      << "Message: " << (worker_report ? worker_report : "None") << "\n\n"
                      << "Photo attached: " << (worker_media ? worker_media : "None") << "\n\n";
        } else {
            std::string worker_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            const char* violationcomment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            const char* violationtimestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const char* worker_report = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            const char* worker_media = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

            std::cout << "Task given: " << desc << "\n\n"
                      << "Status: " << status << "\n\n"
                      << "Violation Comment: " << (violationcomment ? violationcomment : "None") << "\n\n"
                      << "Violation Timestamp: " << (violationtimestamp ? violationtimestamp : "None") << "\n\n"
                      << "Message: " << (worker_report ? worker_report : "None") << "\n\n"
                      << "Photo attached: " << (worker_media ? worker_media : "None") << "\n\n";
        }

        taskNumber++;
    }

    sqlite3_finalize(stmt);
}

/// @brief Get the user ID based on username and password.
/// @param db SQLite DB.
/// @param username User's name.
/// @param password User's password.
/// @return User ID or -1 if not found.
int User::getUserId(sqlite3* db, const std::string& username, const std::string& password) {
    const char* sql = "SELECT id FROM users WHERE username = ? AND password = ?;";
    sqlite3_stmt* stmt;
    string hashed = hashPassword(password);
    int userId = -1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            userId = sqlite3_column_int(stmt, 0);
        } else {
            std::cerr << "User not found while retrieving ID.\n";
        }
    } else {
        std::cerr << "Failed to prepare statement for getting user ID: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
    return userId;
}

/// @brief Display all safety rules from the database.
void User::viewRules(sqlite3* db) {
    std::string query = "SELECT rule_text, timestamp FROM rules;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "\n--- Safety Rules ---\n";
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* rule = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::cout << "Rule: " << rule << timestamp;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

/// @brief Display feedback (if any) for each rule in the system.
void User::ViewRuleFeedback(sqlite3* db) {
    const char* sql = "SELECT id, rule_text, feedback FROM rules";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    std::cout << "\n--- Feedback for Rules ---\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* ruleText = sqlite3_column_text(stmt, 1);
        const unsigned char* feedback = sqlite3_column_text(stmt, 2);

        std::cout << "Rule ID: " << id << "\n";
        std::cout << "Rule: " << (ruleText ? reinterpret_cast<const char*>(ruleText) : "") << "\n";
        std::cout << "Feedback: " << (feedback ? reinterpret_cast<const char*>(feedback) : "No feedback yet.") << "\n";
        std::cout << "-------------------------------------\n";
    }

    sqlite3_finalize(stmt);
}
