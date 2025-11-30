/*
	WELCOME : )
	YOU MUST DO NOT REMOVE EXISTING CODE
	KEEP THE CODE AND ADD YOUR CODE TO THE SECTION MARKED AS DIY
	DO NOT REMOVE PRINT STATEMENT (e.g, std::cout ...)

	Jonghyeok Park
	jonghyeok_park@korea.ac.kr
*/

#include "system.h"
#include "lock.h"

lock::lock() {}

#define LOCK_NAME(lockType) \
    ((lockType) == LOCK_TYPE::SHARED ? "S-LOCK" : \
    (lockType) == LOCK_TYPE::EXCLUSIVE ? "X-LOCK" : "UNKNOWN")

#define OP_NAME(op) \
    ((op) == OP::READ ? "READ" : \
    (op) ==  OP::WRITE ? "WRITE" : "UNKNOWN")


void lock::print_lock_list() {
    for (auto iter = lock_list.begin(); iter != lock_list.end(); ) {
        auto obj = iter->first;
        auto lock_info = iter->second;
        std::cout << "--- OBJECT: " << obj.name << "---\n";
        for (auto info : lock_info) {
            std::cout << LOCK_NAME(info.second) << " acquired by TX" << info.first.id << "\n";
        }
        std::cout << "--------------------\n";

        ++iter;
    }
}

/*
	Acquire lock to the given object with lock type following these steps.

	step1. Find the existing locks on the object and traverse all locks
	step2. Check whether the transaction can acuire the requested lock.
		-- If the object is already locked by a younger transaction and there is a conflict, we will abort (rollback) the younger transaction.
	step3. If the transaction can acquire lock, it adds lock list 
	step4. If the transaction can not acuqire lock then we need to call rollback function.

	[HINT] 
	- You need to see the `lock_list` data structure at `system.h` file.
	```
	// case1. find the all locks in lock_list for given object (e.g., obj). 
		
		auto it = lock_list.find(obj);
		if (it != lock_list.end()) {
			// it means we found something ... 
		}

	// case2. insert data into lock_list upon transaction cna acquire the lock

		lock_list[obj].push_back(std::pair<trx_t, LOCK_TYPE>(trx,type));
	
	```
	- Lock list does not allow duplicate lock. For example, 
	- We only conisder two types of locks: LOCK_TYPE::SHARED and LOCK_TYPE::EXCLUSIVE
*/
bool lock::acquire_lock(trx_t trx, object_t obj, LOCK_TYPE type) {
	// 기존에 obj에 걸려 있는 lock들을 스냅샷으로 복사해서 검사
	std::vector<std::pair<trx_t, LOCK_TYPE>> existing;
	auto it = lock_list.find(obj);
	if (it != lock_list.end()) {
		existing = it->second; // rollback에서 lock_list가 바뀌어도 안전하게 보기 위해 복사
	}

	// 자기 자신이 이미 갖고 있는 lock 여부/종류 체크
	bool already_has_lock = false;
	bool same_type = false;
	bool need_upgrade = false; // S → X 업그레이드

	for (auto& p : existing) {
		if (p.first == trx) {
			already_has_lock = true;
			if (p.second == type) {
				same_type = true;          // 이미 동일한 lock 보유
			}
			else {
				// 여기서는 S 보유, X 요청인 경우만 온다고 가정
				need_upgrade = true;
			}
			break;
		}
	}

	// 두 lock 사이의 conflict 여부 (다른 트랜잭션 기준)
	auto is_conflict = [](LOCK_TYPE held, LOCK_TYPE req) -> bool {
		// S-S 는 compatible, 나머지는 conflict
		if (held == LOCK_TYPE::SHARED && req == LOCK_TYPE::SHARED) return false;
		return true;
		};

	bool can_acquire = true;

	// step1 & step2: 기존 lock들과 충돌 여부 확인 + wound-wait 룰 적용
	for (auto& p : existing) {
		trx_t other_trx = p.first;
		LOCK_TYPE held_type = p.second;

		// 자기 자신은 이미 위에서 처리했으므로 건너뜀
		if (other_trx == trx) continue;

		// 충돌 안 나면 상관 없음
		if (!is_conflict(held_type, type)) continue;

		// 여기서부터는 진짜 conflict
		if (trx < other_trx) {
			// 현재 trx 가 더 old → younger 를 죽인다(rollback)
			rollback(other_trx);
			// rollback 이 lock_list에서 other_trx 의 lock 을 제거해 줄 것이라 가정
		}
		else {
			// 현재 trx 가 younger → 본인이 죽어야 함
			rollback(trx);
			can_acquire = false;
			break;
		}
	}

	if (!can_acquire) {
		// lock 못 얻었으므로 false
		print_lock_list();
		return false;
	}

	// step3: lock 획득 또는 업그레이드
	if (same_type) {
		// 이미 같은 lock 을 갖고 있는 경우: 아무것도 안 하고 성공 처리
		// (중복 삽입 금지)
	}
	else if (need_upgrade) {
		// S → X 업그레이드: 실제 lock_list에서 해당 엔트리를 찾아 타입만 변경
		auto& lock_vec = lock_list[obj]; // 존재하지 않으면 자동 생성되지만, 이미 있다고 보는게 정상
		for (auto& p : lock_vec) {
			if (p.first == trx) {
				p.second = type; // EXCLUSIVE 로 승격
				break;
			}
		}
	}
	else {
		// 새로 lock 을 잡는 경우: 중복 없다고 가정하고 push_back
		lock_list[obj].push_back(std::make_pair(trx, type));
	}

	print_lock_list();
	return true;
}




/*
	Traverse the lock_list and remove all locks acquired by given transaction.
*/

void lock::release_lock(trx_t trx) {
	std::cout << "[LOCK RELEASE] trx" << trx.id << "\n";
	std::cout << "Before releasing locks (lock list status): \n";
	print_lock_list();

	// DIY
	for (auto it = lock_list.begin(); it != lock_list.end(); ) {
		auto& locks = it->second;  // vector/list of (trx_t, LOCK_TYPE)

		// 해당 object에 걸린 lock들 중에서 trx가 잡은 lock만 제거
		for (auto lit = locks.begin(); lit != locks.end(); ) {
			if (lit->first.id == trx.id) {   // 같은 트랜잭션이면 제거
				lit = locks.erase(lit);
			}
			else {
				++lit;
			}
		}

		// 이 object에 더 이상 lock이 없으면 map에서 통째로 제거
		if (locks.empty()) {
			it = lock_list.erase(it);
		}
		else {
			++it;
		}
	}

	std::cout << "After releasing locks (lock list status): \n";
	print_lock_list();
}


/*
	Rollback the transaction.
	step1. Release all locks held by the transaction.
	step2. Remove the actions performed by the transaction from the `output` vector.
	setp3. Remove the remaining actions of the transaction being rolled back from the `actions` vector
			, and then append the actions of the rollbacked transaction to the end of the actions vector. 
			For example, if the actions of T1 (i.e., rollbacked trx) are R1(A), R1(B), and W1(C), then R1(A), R1(B), and W1(C) should be re-executed.
	step4. Reset the timestamp (YOU DO NOT NEED TO MODIFY) 

	[HINT] check `actions` vector, we record transaction's actions in the `trx.actions` vector.
*/

void lock::rollback(trx_t trx) {
	std::cout << "[ROLLBACK] TX" << trx.id << " is rollbeck!\n";

	// step1. Release all locks held by the transaction.
	release_lock(trx);

	// step2. Remove the actions performed by the transaction from the `output` vector.
	for (auto it = output.begin(); it != output.end(); ) {
		std::string& op = *it;
		// 액션 포맷이 R1(A), W2(B), C1 이런 형식이라고 가정
		int tid = (int)(op[1] - '0');
		if (tid == trx.id) {
			it = output.erase(it);
		}
		else {
			++it;
		}
	}

	// step3-1. Remove remaining actions of this transaction from global `actions` vector.
	for (auto it = actions.begin(); it != actions.end(); ) {
		std::string& op = *it;
		int tid = (int)(op[1] - '0');
		if (tid == trx.id) {
			it = actions.erase(it);
		}
		else {
			++it;
		}
	}

	// step3-2. Append all actions of this transaction to the end of `actions` vector.
	// trx.actions 에는 이 트랜잭션의 전체 액션 문자열들이 들어 있다고 가정
	for (auto& act : trx.actions) {
		actions.push_back(act);
	}

	// DO NOT MODIFY
	trx.timestamp = global_counter++;
}




/*
	Process the given action. 
	For example, if the action is R1(A), then execute(trx 1, object A, Read) is called.
	A read operation must acquire LOCK_TYPE::SHARED, while a write operation must acquire LOCK_TYPE::EXCLUSIVE. 
	Neither operation performs the actual read or write; they only acquire the lock. 
	In this case, if the lock is successfully obtained, it returns STATUS::SUCCESS; if not, it returns STATUS::BLOCKED.
	If it returns STATUS::SUCCESS You have to add action into `output` vector (see COMMIT case).
	Upon commit, all locks held by the transaction are released, and the action is added to the output vector. Then, STATUS::COMMIT is returned.
*/
 
STATUS lock::execute(std::string action, trx_t trx, object_t obj) {
	char opcode = action[0];
	OP op;
	switch (opcode) {
	case 'R': op = OP::READ; break;
	case 'W': op = OP::WRITE; break;
	case 'C': op = OP::COMMIT; break;
	default:  op = OP::NONE;  break;
	};

	if (op == OP::READ || op == OP::WRITE) {
		std::cout << "[" << OP_NAME(op) << "]" << " TRX" << trx.id << " OBJECT: " << obj.name << "\n";
		LOCK_TYPE type = (op == OP::READ) ? LOCK_TYPE::SHARED : LOCK_TYPE::EXCLUSIVE;

		// DIY
		bool ok = acquire_lock(trx, obj, type);
		if (ok) {
			// 락을 성공적으로 획득한 경우 output 에 액션 기록
			output.push_back(action);
			return STATUS::SUCCESS;
		}
		else {
			// 락을 못 잡은 경우 (block 되거나 rollback 된 경우)
			return STATUS::BLOCKED;
		}

	}
	else if (op == OP::COMMIT) {
		// DIY: 커밋 시 보유 락 모두 해제
		release_lock(trx);

		std::cout << "[COMMIT] TRX" << trx.id << "\n";
		output.push_back(action);
		return STATUS::COMMIT;
	}
	else {
		std::cout << "WRONG OPERATION\n";
	}

	return STATUS::NONE;
}


/*
	Perform the actions sequentially. 
	The actions vector is parsed from the transaction schedule. 
	
	[HINT] 
	1. Call the execute() function to perform the actions sequentially. 
	2. The result of the most recently executed action should be stored in the `ret` variable.
	3. The waiting_queue is globally accessible. 
*/
void lock::run() {

	STATUS ret = STATUS::NONE;   // 최근 실행 결과 초기화 꼭 해줘야 합니다.
	for (auto it = actions.begin(); it != actions.end(); ) {
		if (actions.size() == 0) break;

		// Now we can parse action with transaction id (`tid`) and object id (`oid`) 
		std::string op = *it;
		int tid = (int)(op[1] - '0');
		char oid = op[3];

		// step1. If the result of the most recently executed action is a transaction commit, 
		// first iterate through the waiting_queue and execute the actions (i.e., call the execute() function).
		// If the return value is STATUS::BLOCKED, put it back into the waiting_queue.

		// DIY
		if (ret == STATUS::COMMIT) {
			std::vector<std::string> next_waiting;

			for (auto& wop : waiting_queue) {
				int wtid = (int)(wop[1] - '0');
				char woid = wop[3];

				STATUS wret = execute(wop, trx_map[wtid], obj_map[woid]);
				ret = wret; // “가장 최근에 실행된 액션의 결과”를 ret에 유지

				if (wret == STATUS::BLOCKED) {
					next_waiting.push_back(wop);  // 아직도 막혔으면 다시 대기
				}
			}

			waiting_queue = std::move(next_waiting);
		}

		// step2. For a read/write transaction, check if the transaction is already present in the waiting queue.
		// If the transaction exists in the waiting_queue, set `blocked` variable to true.
		// This indicates that current transaction can not proceed in this round.
		// Refer to step4, we can re-run the remaining actions in the waiting_list.
		bool blocked = false;
		// DIY
		char opcode = op[0];
		if (opcode == 'R' || opcode == 'W') {
			for (auto& wop : waiting_queue) {
				int wtid = (int)(wop[1] - '0');
				if (wtid == tid) {
					// 이 트랜잭션은 이미 이전 액션에서 BLOCKED 되어 waiting_queue에 있음
					blocked = true;
					// 현재 액션도 나중에 다시 실행해야 하므로 waiting_queue에 넣어 둔다.
					waiting_queue.push_back(op);
					break;
				}
			}
		}

		// step3. If blocked is false, call the execute() function for the given action. If the transaction is blocked, insert it into the waiting_list for future processing.
		// Here, YOU DO NOT HAVE TO MODIFY THE CODE. 
		if (!blocked) {
			ret = execute(op, trx_map[tid], obj_map[oid]);
			if (ret == STATUS::BLOCKED) {
				waiting_queue.push_back(op);
			}

			print_lock_list();
		}

		// DO NOT MODIFY
		it = actions.erase(it);
	}

	/* DO NOT MODIFY */
	/* =========================================================================== */
	// step4. if waiting queue is not empty; we need to re-run
	if (waiting_queue.size() != 0) {
		actions.clear();
		actions.insert(actions.end(), waiting_queue.begin(), waiting_queue.end());
		waiting_queue.clear();
		run();

	}
	else {
		// print final output
		std::cout << "====== final state ======\n";
		for (auto& o : output) {
			std::cout << o << " ";
		}
		std::cout << "\n";
	}
	/* =========================================================================== */

}
