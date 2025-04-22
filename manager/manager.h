#ifndef MANAGER_H_
#define MANAGER_H_

#include <sqlite3.h>
#include <string>
#include "/home/irs-training-pc-2/Desktop/Ehssystem/user/user.h"

/**
 * @class Manager
 * @brief Represents a manager in the EHS management system.
 * 
 * Inherits from the User class. A manager can:
 * - Assign tasks to workers
 * - Report safety violations
 * - Add or delete safety rules
 * - Delete tasks
 */
class Manager : public User {
 public:
  /**
   * @brief Constructs a Manager object.
   */
  Manager();

  /**
   * @brief Destroys the Manager object.
   */
  ~Manager();

  /**
   * @brief Assigns a task to a worker.
   *
   * Displays a list of available workers, prompts the manager
   * for task details, and inserts the task into the database.
   *
   * @param db Pointer to the SQLite database connection.
   */
  void assignTask(sqlite3* db);

  /**
   * @brief Reports a safety violation associated with a task.
   *
   * Lists assigned tasks, allows selection, and records a violation
   * in the task entry with updated status.
   *
   * @param db Pointer to the SQLite database connection.
   */
  void reportViolation(sqlite3* db);

  /**
   * @brief Adds a new safety rule to the system.
   *
   * Prompts the manager for rule text and inserts it into the database.
   *
   * @param db Pointer to the SQLite database connection.
   */
  void addRule(sqlite3* db);

  /**
   * @brief Deletes an existing safety rule.
   *
   * Displays all rules and allows the manager to delete by rule ID.
   *
   * @param db Pointer to the SQLite database connection.
   */
  void deleteRule(sqlite3* db);

  /**
   * @brief Deletes an existing task.
   *
   * Displays all tasks and allows the manager to delete by task ID.
   *
   * @param db Pointer to the SQLite database connection.
   */
  void deleteTask(sqlite3* db);
};

#endif  // MANAGER_H_
