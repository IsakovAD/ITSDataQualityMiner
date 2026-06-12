//adapted from claude code

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

#include <algorithm>


class CCDBDataBase {
public:

    explicit CCDBDataBase(const std::string& db_path){
         //to-do: do we really need ccdb_ object as part of this class?

        std::cout<<"Data base at: "<< db_path <<std::endl;
        if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK)
            throw std::runtime_error("Cannot open DB: " + std::string(sqlite3_errmsg(db_)));

        //to-do: make it configurable
        std::cout<<"creating table!: "<<std::endl;
        createTable();
        
        std::cout<<"done, we have DB with "<< getCount() << " entries" <<std::endl;

    }

    ~CCDBDataBase() { sqlite3_close(db_); }


   long getCount() {
    const char* sql = "SELECT COUNT(*) FROM timestamps;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    
    long result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = sqlite3_column_int64(stmt, 0);
    
    sqlite3_finalize(stmt);
    return result;
}

void upsert(const std::string& run,
            const std::string& apass,
            const std::string& module_name,
            long timestamp,
            long created) {

    std::string module_name_ = module_name;
    std::transform(module_name_.begin(), module_name_.end(),
                   module_name_.begin(), ::tolower);

    const char* sql =
        "INSERT INTO timestamps (run_number, apass, module_name, timestamp, created) "
        "VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(run_number, apass, module_name) DO UPDATE SET "
        "  timestamp = excluded.timestamp, "
        "  created   = excluded.created "
        "WHERE excluded.created > timestamps.created;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[upsert] prepare failed: " << sqlite3_errmsg(db_) << "\n";
        return;
    }
    sqlite3_bind_text (stmt, 1, run.c_str(),          -1, SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, apass.c_str(),        -1, SQLITE_STATIC);
    sqlite3_bind_text (stmt, 3, module_name_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, timestamp);
    sqlite3_bind_int64(stmt, 5, created);

    if (sqlite3_step(stmt) != SQLITE_DONE)
        std::cerr << "[upsert] step failed: " << sqlite3_errmsg(db_) << "\n";

    sqlite3_finalize(stmt);
}


long getNewestTimestamp(const std::string& module) {
    std::string module_ = module;
    std::transform(module_.begin(), module_.end(), module_.begin(), ::tolower);

    const char* sql = "SELECT MAX(created) FROM timestamps WHERE module_name=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(std::string("getNewestTimestamp prepare failed: ")
                                 + sqlite3_errmsg(db_));
    sqlite3_bind_text(stmt, 1, module_.c_str(), -1, SQLITE_STATIC);

    long result = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW &&
        sqlite3_column_type(stmt, 0) != SQLITE_NULL)   // MAX on empty set = NULL
        result = sqlite3_column_int64(stmt, 0);

    sqlite3_finalize(stmt);
    return result;   // -1 = nothing stored for this module yet
}

void setNewestTimestamp(const std::string& module, long created) {
    const char* sql =
        "INSERT INTO sync_cursor (module_name, last_created) VALUES (?, ?) "
        "ON CONFLICT(module_name) DO UPDATE SET last_created=excluded.last_created "
        "WHERE excluded.last_created > sync_cursor.last_created;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[setNewestTimestamp] prepare failed: " << sqlite3_errmsg(db_) << "\n";
        return;
    }
    sqlite3_bind_text(stmt, 1, module.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, created);
    if (sqlite3_step(stmt) != SQLITE_DONE)
        std::cerr << "[setNewestTimestamp] step failed: " << sqlite3_errmsg(db_) << "\n";
    sqlite3_finalize(stmt);
}


    // ── Query a single timestamp ───────────────────────────────────
    long getTimestamp(const std::string& run,
                      const std::string& apass,
                      const std::string& module_name) {


        string module_name_ = module_name;
        std::transform(module_name_.begin(), module_name_.end(), module_name_.begin(), ::tolower);

        std::cout<<"[CCDBDataBase] downloading timestamp for run: " <<run << " apass= "<< apass << " module: "<< module_name_ <<std::endl;

        const char* sql =
            "SELECT timestamp FROM timestamps "
            "WHERE run_number=? AND apass=? AND module_name=?;";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, run.c_str(),         -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, apass.c_str(),       -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, module_name_.c_str(), -1, SQLITE_STATIC);

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
    // void createTable() {
    //     const char* sql =
    //         "CREATE TABLE IF NOT EXISTS timestamps ("
    //         "  run_number  TEXT NOT NULL,"
    //         "  apass       TEXT NOT NULL,"
    //         "  module_name TEXT NOT NULL,"
    //         "  timestamp   INTEGER NOT NULL,"
    //         "  PRIMARY KEY (run_number, apass, module_name)"
    //         ");";

    //     char* err = nullptr;
    //     if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
    //         std::string msg = err;
    //         sqlite3_free(err);
    //         throw std::runtime_error("Failed to create table: " + msg);
    //     }
    // }


void createTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS timestamps ("
        "  run_number  TEXT NOT NULL,"
        "  apass       TEXT NOT NULL,"
        "  module_name TEXT NOT NULL,"
        "  timestamp   INTEGER NOT NULL,"   // validity → used to download
        "  created     INTEGER NOT NULL,"   // upload time → freshness key
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