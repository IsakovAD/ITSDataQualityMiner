//adapted from claude code

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>



class CCDBDataBase {
public:

    explicit CCDBDataBase(const std::string& db_path){
         //to-do: do we really need ccdb_ object as part of this class?


        if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK)
            throw std::runtime_error("Cannot open DB: " + std::string(sqlite3_errmsg(db_)));

        //to-do: make it configurable
        std::cout<<"creating table!: "<<std::endl;
        createTable();
        
        std::cout<<"done"<<std::endl;

    }

    ~CCDBDataBase() { sqlite3_close(db_); }

    void upsert(const std::string& run,
                const std::string& apass,
                const std::string& module_name,
                long timestamp) {

        const char* sql =
            "INSERT INTO timestamps (run_number, apass, module_name, timestamp) "
            "VALUES (?, ?, ?, ?) "
            "ON CONFLICT(run_number, apass, module_name) "
            "DO UPDATE SET timestamp=excluded.timestamp;";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, run.c_str(),         -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, apass.c_str(),       -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, module_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 4, timestamp);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            std::cerr << "Upsert failed: " << sqlite3_errmsg(db_) << "\n";

        sqlite3_finalize(stmt);
    }


    long getNewestTimestamp(const std::string& module)
    {
        const char* sql = "SELECT MAX(timestamp) FROM timestamps WHERE module_name=?;";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }
        sqlite3_bind_text(stmt, 1, module.c_str(),         -1, SQLITE_STATIC);


        long result = -1;
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                result = sqlite3_column_int64(stmt, 0);
            }
            // else: table is empty → result stays std::nullopt
        } else {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute statement");
        }

        sqlite3_finalize(stmt);
        return result;
    }

    // ── Query a single timestamp ───────────────────────────────────
    long getTimestamp(const std::string& run,
                      const std::string& apass,
                      const std::string& module_name) {

        const char* sql =
            "SELECT timestamp FROM timestamps "
            "WHERE run_number=? AND apass=? AND module_name=?;";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, run.c_str(),         -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, apass.c_str(),       -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, module_name.c_str(), -1, SQLITE_STATIC);

        long result = -1;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            result = sqlite3_column_int64(stmt, 0);
        else
            std::cerr << "Not found: run=" << run 
                      << " apass=" << apass 
                      << " path=" << module_name << "\n";

        sqlite3_finalize(stmt);
        return result;  // -1 means not found
    }

    // ── Query all timestamps for a run ────────────────────────────
    std::vector<std::pair<std::string, long>> 
    getAllForRun(const std::string& run, const std::string& apass) {

        const char* sql =
            "SELECT module_name, timestamp FROM timestamps "
            "WHERE run_number=? AND apass=? "
            "ORDER BY module_name;";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, run.c_str(),   -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, apass.c_str(), -1, SQLITE_STATIC);

        std::vector<std::pair<std::string, long>> results;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            long ts          = sqlite3_column_int64(stmt, 1);
            results.emplace_back(path, ts);
        }

        sqlite3_finalize(stmt);
        return results;
    }

    // ── Check if entry exists (to skip re-fetching from CCDB) ─────
    bool exists(const std::string& run,
                const std::string& apass,
                const std::string& module_name) {
        return getTimestamp(run, apass, module_name) != -1;
    }

private:
    sqlite3* db_ = nullptr;
    void createTable() {
        const char* sql =
            "CREATE TABLE IF NOT EXISTS timestamps ("
            "  run_number  TEXT NOT NULL,"
            "  apass       TEXT NOT NULL,"
            "  module_name TEXT NOT NULL,"
            "  timestamp   INTEGER NOT NULL,"
            "  PRIMARY KEY (run_number, apass, module_name)"
            ");";

        char* err = nullptr;
        if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
            std::string msg = err;
            sqlite3_free(err);
            throw std::runtime_error("Failed to create table: " + msg);
        }
    }
};