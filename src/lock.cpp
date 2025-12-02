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
    // step1. 해당 object 의 기존 lock 들 찾기
    auto it = lock_list.find(obj);
	    // std::cout << "\n[DEBUG] acquire_lock req T" << trx.id
        //       << " obj=" << obj.name
        //       << " type=" << (type == LOCK_TYPE::SHARED ? "S" : "X") << "\n";
    if (it == lock_list.end()) {
        // 아무도 안 잡고 있으면 바로 획득
        lock_list[obj].push_back(std::make_pair(trx, type));
        return true;
    }

    auto &locks = it->second;
    // std::cout << "[DEBUG]  -> existing locks on " << obj.name << ":\n";
    // for (auto &p : locks) {
    //     std::cout << "    holder T" << p.first.id
    //               << " type=" << (p.second == LOCK_TYPE::SHARED ? "S" : "X")
    //               << " ts=" << p.first.timestamp << "\n";
    // }
    // 이 트랜잭션이 이미 들고 있는지 확인 (업그레이드 대비)
    bool has_own_lock = false;
    LOCK_TYPE own_type = LOCK_TYPE::SHARED;

    for (auto &p : locks) {
        if (p.first.id == trx.id) {
            has_own_lock = true;
            own_type = p.second;
            break;
        }
    }

    // 이미 같은 타입의 락을 들고 있으면 더 할 일 없음
    if (has_own_lock && own_type == type) {
        return true;
    }

    auto is_conflict = [](LOCK_TYPE held, LOCK_TYPE req) {
        // S-S 만 호환, 그 외는 전부 충돌
        if (held == LOCK_TYPE::SHARED && req == LOCK_TYPE::SHARED) return false;
        return true;
    };

    bool conflict_with_older = false;
    std::vector<trx_t> victims;   // younger 들 – wound 대상

    // step2. 충돌 확인 + wound-wait 정책 적용
    for (auto &p : locks) {
        trx_t holder = p.first;
        LOCK_TYPE held_type = p.second;

        if (holder.id == trx.id) continue; // 자기 자신은 패스

        if (!is_conflict(held_type, type)) continue;

		// 바로 여기!
        // std::cout << "[DEBUG] conflict: req T" << trx.id
        //           << " (ts=" << trx.timestamp
        //           << "), holder T" << holder.id
        //           << " (ts=" << holder.timestamp << ")\n";

        // trx 가 older 인 경우: younger holder 롤백 (wound-wait)
        if (trx.timestamp < holder.timestamp) {
            victims.push_back(holder);
        } else {
            // holder 가 older → 우리는 기다려야 함 (block)
            conflict_with_older = true;
        }
    }

    // younger 들 먼저 롤백 (release_lock 까지 처리됨)
    for (auto &v : victims) {
        rollback(v);
    }

    // older 와의 충돌이 남아 있으면 지금은 못 잡음 → BLOCK
    if (conflict_with_older) {
        return false;
    }

    // 여기까지 왔으면 충돌 없음 → lock 부여 / 업그레이드
    auto &final_locks = lock_list[obj];

    if (has_own_lock) {
        // S → X 업그레이드
        for (auto &p : final_locks) {
            if (p.first.id == trx.id) {
                p.second = type;
                break;
            }
        }
    } else {
        final_locks.push_back(std::make_pair(trx, type));
    }

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

		// �ش� object�� �ɸ� lock�� �߿��� trx�� ���� lock�� ����
		for (auto lit = locks.begin(); lit != locks.end(); ) {
			if (lit->first.id == trx.id) {   // ���� Ʈ������̸� ����
				lit = locks.erase(lit);
			}
			else {
				++lit;
			}
		}

		// �� object�� �� �̻� lock�� ������ map���� ��°�� ����
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
    //  -> 이때만 "R1(A) R2(B) erase R2(B)" 같은 로그를 찍음.
    std::string prev = "";
    for (auto it = output.begin(); it != output.end(); ) {
        const std::string &cur = *it;

        if (cur.size() >= 2 && (int)(cur[1] - '0') == trx.id) {
            // 바로 앞에 남아 있는 액션이 있다면, 그걸 prev로 찍어 줌
            if (!prev.empty()) {
                std::cout << prev << " " << cur << " erase " << cur << "\n";
            }
            it = output.erase(it);  // 현재 trx의 액션 삭제
        } else {
            // trx가 아닌 다른 트랜잭션의 액션은 유지하고 prev 갱신
            prev = cur;
            ++it;asd
        }
    }

    // step3-1. Remove the remaining actions of this transaction from the `actions` vector.
    //  -> 여기서는 절대 로그 안 찍음 (솔루션도 안 찍음)
    for (auto it = actions.begin(); it != actions.end(); ) {
        const std::string &cur = *it;
        if (cur.size() >= 2 && (int)(cur[1] - '0') == trx.id) {
            it = actions.erase(it);
        } else {
            ++it;ssddaadddd
        }
    }
asdasfsda
    // step3-2. Append all actions of this transaction to the end of `actions` vector.
    //  -> 스케줄 처음부터 다시 실행해야 하니까 trx.actions 전체를 뒤에 붙임
    for (const auto &act : trx.actions) {
        actions.push_back(act);
    }

    // step4. DO NOT MODIFY
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
	switch (opcode){
		case 'R': op = OP::READ; break;
		case 'W': op = OP::WRITE; break;
		case 'C': op = OP::COMMIT; break;
		default:  op = OP::NONE;  break;
	}; 

	if (op == OP::READ || op == OP::WRITE) {
		std::cout << "[" << OP_NAME(op) << "]"
		          << " TRX" << trx.id
		          << " OBJECT: " << obj.name << "\n";

		LOCK_TYPE type = (op == OP::READ)
		               ? LOCK_TYPE::SHARED
		               : LOCK_TYPE::EXCLUSIVE;	

		// DIY 시작
		bool ok = acquire_lock(trx, obj, type);
		if (ok) {
			// 락 획득 성공 시: output 에 기록 + success 메세지
			output.push_back(action);
			std::cout << action << " success\n";
			return STATUS::SUCCESS;
		} else {
			std::cout << action << " is blocked\n";
			return STATUS::BLOCKED;
		}
		// DIY 끝

	} else if (op == OP::COMMIT) {
		// DIY: 커밋 시 모든 락 해제
		release_lock(trx);

		std::cout << "[COMMIT] TRX" << trx.id << "\n";
		output.push_back(action);
		return STATUS::COMMIT;
	} else {
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

    STATUS ret = STATUS::NONE;

    for (auto it = actions.begin(); it != actions.end(); ) {
        if (actions.empty()) break;

        std::string op = *it;
        if (op.empty()) {            // 이상한 공백 문자열 방어
            it = actions.erase(it);
            continue;
        }

        char opcode = op[0];         // 'R' / 'W' / 'C'
        int  tid   = -1;
        char oid   = '\0';

        // ----- 안전하게 tid / oid 파싱 -----
        if (opcode == 'R' || opcode == 'W') {
            // 형식: R1(A) / W2(B) 라고 가정 → 최소 길이 4
            if (op.size() < 4) {
                std::cout << "WRONG OP FORMAT: " << op << "\n";
                it = actions.erase(it);
                continue;
            }
            tid = op[1] - '0';
            oid = op[3];
        }
        else if (opcode == 'C') {
            // 형식: C1 → 최소 길이 2
            if (op.size() < 2) {
                std::cout << "WRONG OP FORMAT: " << op << "\n";
                it = actions.erase(it);
                continue;
            }
            tid = op[1] - '0';
            // COMMIT 은 object 를 안 쓰니까, 아무 object 하나 임의로 넘겨줌
            if (!obj_map.empty())
                oid = obj_map.begin()->first;
        }
        else {
            std::cout << "WRONG OP FORMAT: " << op << "\n";
            it = actions.erase(it);
            continue;
        }
        // -------------------------------

        // step1. 직전에 COMMIT 이었으면 waiting_queue 먼저 처리
        if (ret == STATUS::COMMIT) {
            std::vector<std::string> next_waiting;

            for (auto &wop : waiting_queue) {
                if (wop.empty()) continue;

                char wopcode = wop[0];
                int  wtid    = -1;
                char woid    = '\0';

                if (wopcode == 'R' || wopcode == 'W') {
                    if (wop.size() < 4) continue;
                    wtid = wop[1] - '0';
                    woid = wop[3];
                }
                else if (wopcode == 'C') {
                    if (wop.size() < 2) continue;
                    wtid = wop[1] - '0';
                    if (!obj_map.empty())
                        woid = obj_map.begin()->first;
                }
                else {
                    continue;
                }

                STATUS wret = execute(wop, trx_map[wtid], obj_map[woid]);
                if (wret == STATUS::BLOCKED)
                    next_waiting.push_back(wop);

                ret = wret;
            }

            waiting_queue = std::move(next_waiting);
        }

        // step2. R/W 인데 이미 waiting_queue 에 같은 tid 가 있으면 막기
        bool blocked = false;
            for (auto &wop : waiting_queue) {
                if (wop.size() < 2) continue;
                int wtid = wop[1] - '0';
                if (wtid == tid) {
                    blocked = true;
                    // 나중에 다시 실행해야 하니까 큐에 넣어둠
                    waiting_queue.push_back(op);
                    break;
                }
            }

        // step3. 원래 스켈레톤 로직
        if (!blocked) {
            ret = execute(op, trx_map[tid], obj_map[oid]);
            if (ret == STATUS::BLOCKED)
                waiting_queue.push_back(op);

            print_lock_list();
        }

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

    } else {
        // print final output
        std::cout << "====== final state ======\n";
        for (auto &o : output) {
            std::cout << o << " ";
        }
        std::cout << "\n";	
    }
    /* =========================================================================== */	
}

