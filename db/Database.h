#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

/**
 * @class DatabaseManager
 * @brief Manages SQLite database operations including table setup and database connection.
 *
 * This class handles the initialization, destruction, and management of the SQLite database,
 * as well as setting up necessary tables such as users, tasks, and rules.
 */
class DatabaseManager {
private:
    sqlite3* db; ///< Pointer to the SQLite database connection

public:
    /**
     * @brief Constructor that opens the SQLite database.
     * 
     * @param dbName The name of the SQLite database file.
     */
    DatabaseManager(const std::string& dbName);

    /**
     * @brief Destructor that closes the SQLite database.
     */
    ~DatabaseManager();

    /**
     * @brief Retrieves the SQLite database connection.
     * 
     * @return sqlite3* The SQLite database connection pointer.
     */
    sqlite3* getDB();

    /**
     * @brief Sets up the required tables in the database.
     * 
     * This function creates the 'users', 'tasks', and 'rules' tables if they do not already exist.
     * It ensures the necessary schema is in place for the application to function properly.
     */
    void setupTables();
};

#endif // DATABASE_H
