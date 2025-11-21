/* 
	WELCOME : )
	YOU MUST DO NOT REMOVE EXISTING CODE
	KEEP THE CODE AND ADD YOUR CODE TO THE SECTION MARKED AS DIY
	DO NOT REMOVE PRINT STATEMENT (e.g, std::cout ...)

	Jonghyeok Park
	jonghyeok_park@korea.ac.kr
*/

#include "system.h"
#include "occ.h"

occ::occ(){}

/*
	Read data item and records it into transation's private read set
	[HINT] you can access transaction's read/write set like calling this;
	```
		trx.read_set
	``` 
	In C++, "set" data structures are designed to prevent duplicates.
	This function does not allow to store duplicate elements.
*/
void occ::trx_read(trx_t& trx, object_t obj) {
	// increment global timestamp (DO NOT REMOVE)
	occ_timestamp++;
	std::cout << "[READ] trx" << trx.id << " read object: " << obj.name << "\n";

	// DIY
}

/*
	Write data item and records it into transaction's private write set.
	This function does NOT write to the main database file.
	[HINT] see `trx_read` function.
*/
void occ::trx_write(trx_t& trx, object_t obj) {
	occ_timestamp++;
	std::cout << "[LOCAL WRITE] trx" << trx.id << " write object: " << obj.name << "\n";

	// DIY
}


/*
	Validate the transaction.
	When transaction try to commit operation; we first call this function (see the `run()` function.)
	[HINT] compare the timestamp (i.e., start, finish, validate timestamp) and check all the transaction's write_set. You can access write set of all transaction using this code statement.
	For transaction structure, you can see the `system.h` file.
	```
		for (auto it = trx_map.begin(); it != trx_map.end(); ++it) {
			trx_t tx = it->second;
			... 
			tx.write_set
		}
		
	```
*/
bool occ::trx_validate(trx_t& trx) {
	occ_timestamp++;
	std::cout << "[VALIDATION] trx:" << trx.id << " enter validation phase \n";
	// DIY
	// you can change `return true`; I added this statement for compilation purposes.
	return true;
}

/*
	Commit the transaction
	[HIN] YOU ONLY HAVE TO UPDATE THE TIMESTAMP (EITHER finish, validate or start TIMESTAMP).
*/
void occ::commit(trx_t& trx) {
	occ_timestamp++;
	std::cout << "[COMMIT] transaction tx" << trx.id << "\n";
	// DIY
}

////////////////////////////////////////////////////////////////////////////////////////////
/*
	Don't worry! You don't need to add code in the `abort()`, `write()`, and `run()` functions.
	BUT you have to understand how our program works.
*/


/*
	Copy all data objects from transaction's local write set to main database file.
	Here, we do not actullay writes the data; instead, we print out the name of all the objects. 
*/
void occ::write(trx_t& trx) {
	occ_timestamp++;
	std::cout << "[WRITE] ";
	for (const auto& obj : trx.write_set) {
		std::cout << obj.name << " ";
	}
	std::cout << "objects are written to main database file!\n";
	// invoke real commit process 
	commit(trx);
}


/*
	Upon transcation abort, we remove all data objects from its local read/write sets.
	Assign the current timestamp (i.e., occ_timestamp) to the `strat_ts`.
	Reset the validate_ts and finish_ts to INF value (infinite value).
	For aborting transactions, we insert transactrion's actions to the actions vector to redo its operations.
*/
void occ::abort(trx_t& trx) {
	std::cout << "[ABORT] trx" << trx.id <<"\n";
	trx.write_set.clear();
	trx.read_set.clear();
	trx.start_ts = occ_timestamp;
	trx.validate_ts = INF;
	trx.finish_ts = INF;

	// add actions 
	actions.insert(actions.end(), trx.actions.begin(), trx.actions.end());
}

/*
	The `actions` vector stores transaction operations (e.g., R1(A)).
	This function reads the operations performs the appropriate work, and then erase the operation from `actions` vector
*/
void occ::run() {
	for (auto it = actions.begin(); it != actions.end(); ) {
		if (actions.size() == 0) break;
		// now we can parse the opration like 
		// e.g., R1(A); 
		// opcode = 'R' / tid = 1 / oid = 'A'
		std::string op = *it;
		char opcode = op[0];
		int tid = (int)(op[1] - '0');
		char oid = op[3];

		// set start_ts
		if ( trx_map[tid].start_ts == INF) {
			trx_map[tid].start_ts = occ_timestamp;
		}

		// step1. run operation
		if ( opcode == 'R') {
			trx_read(trx_map[tid], obj_map[oid]);
		} else if ( opcode == 'W') {
			trx_write(trx_map[tid], obj_map[oid]);
		} else if (opcode =='C') {
			bool pass = trx_validate(trx_map[tid]);
			if (pass) {
				// here, we invoke write() operation, instead commit() function
				// because it contains commit process.
				write(trx_map[tid]);
			} else {
				abort(trx_map[tid]);
			}	
		} else {
			std::cout << "WRONG OPERATION!\n";	
		}

		// erase actions;
		it = actions.erase(it);
	}

	return ;
}
