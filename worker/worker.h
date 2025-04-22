#ifndef WORKER_H_
#define WORKER_H_

#include <sqlite3.h>
#include <string>
#include "/home/irs-training-pc-2/Desktop/Ehssystem/user/user.h"

/**
 * @class Worker
 * @brief Represents a worker in the EHS management system.
 * 
 * Inherits from the User class. A worker can:
 * - Report task completion with media and description.
 * - Provide feedback on rules.
 */
class Worker : public User {
 public:
  /**
   * @brief Constructs a Worker object.
   */
  Worker();

  /**
   * @brief Destroys the Worker object.
   */
  ~Worker();

  /**
   * @brief Allows a worker to report task work.
   * 
   * Lists assigned tasks, accepts a report description and media path,
   * saves the media, and updates the task in the database.
   *
   * @param db Pointer to the SQLite database connection.
   * @param user_id ID of the worker reporting the task.
   */
  void reportTaskWork(sqlite3* db, int user_id);

  /**
   * @brief Allows a worker to give feedback on a rule.
   *
   * Displays a list of all rules and lets the worker submit feedback
   * which is saved in the corresponding rule entry.
   *
   * @param db Pointer to the SQLite database connection.
   */
  void GiveRuleFeedback(sqlite3* db);
};

#endif  // WORKER_H_
