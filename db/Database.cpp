
/**
 * @file Database.cpp
 * @brief Implementation of the DatabaseManager class for managing SQLite database operations.
 *
 * This file defines the DatabaseManager class responsible for handling database 
 * connections and setting up tables for users, tasks, and rules. It manages 
 * the database connection and ensures that the required tables exist.
 *
 * OOP Principles:
 * - Encapsulation: The database connection and table creation functionality are encapsulated 
 *   in the DatabaseManager class. The class provides public methods to interact with the database.
 * - Constructor and Destructor: The constructor manages opening the database, and the destructor 
 *   ensures proper cleanup by closing the connection.
 *
 */

#include "Database.h"
#include <sqlite3.h>
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& dbName) {
    // Open the SQLite database
    if (sqlite3_open(dbName.c_str(), &db)) {
        std::cerr << "Failed to open DB: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    }
}

/**
 * @brief Destructor for closing the SQLite database connection.
 *
 * This function ensures that the database connection is properly closed
 * when the DatabaseManager object is destroyed.
 */
DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

/**
 * @brief Sets up the required tables in the database.
 *
 * This function creates the 'users', 'tasks', and 'rules' tables if they do not
 * already exist. It will be called during initialization to ensure the database schema is set up.
 */
void DatabaseManager::setupTables() {
    const char* userTable = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "username TEXT UNIQUE, "
                            "password TEXT, "
                            "role TEXT);";

    const char* taskTable = "CREATE TABLE IF NOT EXISTS tasks ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "worker_id TEXT, "
                            "worker_username TEXT, "
                            "task_description TEXT, "
                            "status TEXT, "
                            "violation_comment TEXT, "
                            "violation_timestamp TEXT, "
                            "worker_report TEXT, "
                            "worker_media TEXT, "
                            "FOREIGN KEY(worker_id) REFERENCES users(username));";

    const char* rulesTable = "CREATE TABLE IF NOT EXISTS rules ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "rule_text TEXT NOT NULL, "
                             "feedback TEXT, "
                             "timestamp TEXT);";

    // Execute the queries to create tables
    if (sqlite3_exec(db, userTable, 0, 0, nullptr) != SQLITE_OK) {
        std::cerr << "Error creating users table: " << sqlite3_errmsg(db) << "\n";
    }
    if (sqlite3_exec(db, taskTable, 0, 0, nullptr) != SQLITE_OK) {
        std::cerr << "Error creating tasks table: " << sqlite3_errmsg(db) << "\n";
    }
    if (sqlite3_exec(db, rulesTable, 0, 0, nullptr) != SQLITE_OK) {
        std::cerr << "Error creating rules table: " << sqlite3_errmsg(db) << "\n";
    }
}

/**
 * @brief Gets the SQLite database connection.
 *
 * This function returns the SQLite database connection so that it can
 * be used for querying or other database operations.
 *
 * @return sqlite3* Pointer to the SQLite database connection.
 */
sqlite3* DatabaseManager::getDB() {
    return db; 
}
