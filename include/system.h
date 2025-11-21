#ifndef SYSTEM_H
#define SYSTEM_H

/* DO NOT MODFIY */
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdint>

const int INF = (~0U >> 2);


// READ, WRITE, COMMIT
enum class OP {
    READ,
    WRITE,
    COMMIT,
	NONE
};

enum class LOCK_TYPE {
	SHARED,
	EXCLUSIVE
};

enum class STATUS {
	NONE,
	SUCCESS,
	BLOCKED,
	COMMIT
};

extern uint64_t global_counter;
extern uint64_t occ_timestamp;


// object structure
struct object_t {
    char name;
	int val;
	int w_ts;
	object_t();
    object_t(char n);
	bool operator==(const object_t& other) const;
	bool operator<(const object_t& other) const;
};

// transaction structure
struct trx_t {
    int  id;
    std::vector<std::string> actions;
    bool is_commit;
    bool is_abort;	
	uint64_t timestamp;
	// read, write set are used in OCC only.
    std::set<object_t> read_set;
    std::set<object_t> write_set;

	uint64_t start_ts;
	uint64_t validate_ts;
	uint64_t finish_ts;

	trx_t();
    trx_t(int tid);

	bool operator==(const trx_t& other) const;
	bool operator<(const trx_t& other) const;

};


// read the transaction schedule file and fill the transaction information
void parse_trx_schedule(const std::string& filename);

// action list
extern std::vector<std::string> actions;
// transaction map (key trx_id, value trx)
extern std::map<int, trx_t> trx_map;
// object map (key: object name; value: object)
extern std::map<char, object_t> obj_map;


#endif
