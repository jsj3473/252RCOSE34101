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

	// DIY: 트랜잭션의 private read set에 객체 추가 (중복은 set이 자동으로 제거)
	trx.read_set.insert(obj);
}

/*
	Write data item and records it into transaction's private write set.
	This function does NOT write to the main database file.
	[HINT] see `trx_read` function.
*/
void occ::trx_write(trx_t& trx, object_t obj) {
	occ_timestamp++;
	std::cout << "[LOCAL WRITE] trx" << trx.id << " write object: " << obj.name << "\n";

	// DIY: 트랜잭션의 private write set에 객체 추가
	trx.write_set.insert(obj);
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

	// 1) 이 트랜잭션의 validate 타임스탬프 설정
	trx.validate_ts = occ_timestamp;   // ← 이름은 system.h에 맞춰 수정

	// 2) 다른 트랜잭션들과 충돌 여부 검사
	for (auto it = trx_map.begin(); it != trx_map.end(); ++it) {
		trx_t& other = it->second;

		if (other.id == trx.id) continue; // 자기 자신은 스킵

		// 아직 커밋 안 된 트랜잭션은 검증 대상에서 제외
		// (finish_ts 초기값이 0 또는 -1 같은지 system.h 보고 조건 맞추세요)
		if (other.finish_ts == 0) continue;

		// (1) other 가 trx 시작 전에 이미 끝났으면 겹치는 구간이 없음 → OK
		if (other.finish_ts <= trx.start_ts) {
			continue;
		}

		// (2) other 가 trx 검증 이후에 시작했다면 아직 겹치는 거 없음 → OK
		if (other.start_ts >= trx.validate_ts) {
			continue;
		}

		// 여기까지 왔다는 건 시간적으로 overlap 가능성이 있는 트랜잭션
		// → other.write_set ∩ trx.read_set 가 비어 있는지 체크
		bool conflict = false;
		for (const auto& obj : trx.read_set) {
			if (other.write_set.find(obj) != other.write_set.end()) {
				std::cout << "[VALIDATION FAIL] trx" << trx.id
					<< " conflicts with trx" << other.id
					<< " on object " << obj.name << "\n";
				conflict = true;
				break;
			}
		}

		if (conflict) {
			return false;   // 검증 실패 → 호출 쪽(run)에서 롤백 호출해야 함
		}
	}

	// 여기까지 왔으면 검증 성공
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
		// 커밋 완료 시각만 업데이트
	trx.finish_ts = occ_timestamp;   // 이름은 system.h에 맞춰서 수정
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
