#include "system.h"
#include <map>
#include <fstream>
#include <regex>
#include <iomanip> 

/* DO NOT MODIFY */

trx_t::trx_t() : id(0), is_commit(false), is_abort(false), timestamp(0), start_ts(0) {}
object_t::object_t(){};
trx_t::trx_t(int tid) : id(tid), is_commit(false), is_abort(false), timestamp(0) {
#ifdef OCC
	start_ts = 0;
	validate_ts = INF;
	finish_ts = INF;
#endif
}
object_t::object_t(char name) : name(name), val(0) {};

bool object_t::operator==(const object_t& other) const {
	return name == other.name;
}

bool object_t::operator<(const object_t& other) const {
	return name < other.name;
}

bool trx_t::operator==(const trx_t& other) const {
	return id == other.id;
}

bool trx_t::operator<(const trx_t& other) const {
	return id < other.id;
}

std::vector<std::string> actions;
std::map<int, trx_t> trx_map;
std::map<char, object_t> obj_map;
uint64_t global_counter = 0;
uint64_t occ_timestamp = 0;



inline void print_obj_map() {
    std::cout << "=== Object Map ===" << std::endl;
    for (const auto& [obj, object] : obj_map) {
        std::cout << "[INFO] name: " << obj << " value: " << object.val << std::endl;
    }
    std::cout << "===================" << std::endl;
}

inline void print_trx_map() {
    std::cout << "=== Transaction Map ===" << std::endl;
    for (const auto& [tid, trx] : trx_map) {
        std::cout << "Transaction ID: " << tid << std::endl;
        std::cout << "  Actions: ";
        for (const auto& action : trx.actions) {
            std::cout << action << " ";
        }
        std::cout << std::endl;

        std::cout << "  Committed: " << std::boolalpha << trx.is_commit << std::endl;
        std::cout << "  Aborted: " << std::boolalpha << trx.is_abort << std::endl;
		std::cout << "  Timestamp:" << trx.timestamp << std::endl;

        std::cout << "  Read Set: ";
        for (const auto& obj : trx.read_set) {
            std::cout << obj.name << " ";
        }
        std::cout << std::endl;

        std::cout << "  Write Set: ";
        for (const auto& obj : trx.write_set) {
            std::cout << obj.name << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "========================" << std::endl;
}

// parse the transaction schedule information
// we separate the transaction operation line by line
// file format: [Action][Transaction ID]([OBJECT])
// e.g., R1(A)
void parse_trx_schedule(const std::string& filename) {

    std::ifstream fin(filename);
    std::string line;
    std::regex rw_regex(R"(([RW])(\d+)\((\w)\))");
    std::regex c_regex(R"(C(\d+))");
    std::smatch m;

    while (std::getline(fin, line)) {

		actions.push_back(line);
        if (std::regex_match(line, m, rw_regex)) {
			// here, we only check the format: [ACTION][TRX-ID]([OBJECT]) 
			// e.g., R1(A), W1(B) , ... etc
            int tid = std::stoi(m[2]);
            char obj = m[3].str()[0];

			// create transaction;
			// duplicates are not allowd.
            if (trx_map.count(tid) == 0) {
                trx_map[tid] = trx_t(tid);
				trx_map[tid].timestamp = global_counter;
				global_counter++;
				trx_map[tid].start_ts = INF;
			}

			// craete object
            if (obj_map.count(obj) == 0)
                obj_map[obj] = object_t(obj);

			// here, we record transaction's all actions
			// we can get transaction's action.
			trx_map[tid].actions.push_back(line);

        } else if (std::regex_match(line, m, c_regex)) {
			// here, we only check the format: [ACTION][TRX-ID]
			// e.g., C1 C2  
            int tid = std::stoi(m[1]);
            if (trx_map.count(tid) == 0)
                trx_map[tid] = trx_t(tid);
            trx_map[tid].actions.push_back(line);
            trx_map[tid].is_commit = true;
        }
    }

	// (debug)
	print_obj_map();
	print_trx_map();

	std::cout << "ACTION LIST" << std::endl;
	for (const auto& action : actions) {
		std::cout << action << " ";
    }
	std::cout << std::endl;
 
}
