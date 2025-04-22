#pragma once
#include <string>
#include <sqlite3.h>
using namespace std;

class User {
protected:
    std::string username;
    std::string password;
    std::string role;

public:
    User(const std::string& role);
    User();
    ~User();
    string hashPassword(const string& password);
    bool registerUser(sqlite3* db, const std::string& username, const std::string& password);
    bool loginUser(sqlite3* db, const std::string& username, const std::string& password);
    string getUserRole(sqlite3* db, const std::string& username, const std::string& password);
    bool userExists(sqlite3* db, const std::string& username, const std::string& password);
    void viewTaskDetails(sqlite3* db, int userId, bool isManager = false);
    int getUserId(sqlite3* db, const std::string& username, const std::string& password);
    void viewRules(sqlite3* db);
    void ViewRuleFeedback(sqlite3* db);


};