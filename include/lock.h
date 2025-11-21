#ifndef LOCK_H
#define LOCK_H

#include "system.h"

class lock {
public:
	lock();
	// acquire lock
	bool acquire_lock(trx_t trx, object_t obj, LOCK_TYPE type);

	// release lock
	void release_lock(trx_t trx);

	// rollback all operations of given transaction;
	void rollback(trx_t trx);

	// execute action
	STATUS execute(std::string action, trx_t trx, object_t obj);
	
	// run transcation schedule
	void run();
	
	// lock list
	std::map<object_t, std::vector<std::pair<trx_t, LOCK_TYPE>>> lock_list;	

	// queue
	std::vector<std::string> waiting_queue;
	// output
	std::vector<std::string> output;

	// print lock_list 
	void print_lock_list();
};

#endif
