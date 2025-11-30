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
#include <cctype> // 이미 있을 수도 있음

static void print_rw_sets(const trx_t& trx) {
    std::cout << "[DEBUG] trx" << trx.id << " read_set: ";
    for (const auto& o : trx.read_set) {
        std::cout << o.name << " ";
    }
    std::cout << "\n";

    std::cout << "[DEBUG] trx" << trx.id << " write_set: ";
    for (const auto& o : trx.write_set) {
        std::cout << o.name << " ";
    }
    std::cout << "\n";
}

occ::occ(){}

void occ::trx_read(trx_t& trx, object_t obj) {
    // increment global timestamp (DO NOT REMOVE)
    occ_timestamp++;
    std::cout << "[READ] trx" << trx.id << " read object: " << obj.name << "\n";

    // read_set에 추가
    trx.read_set.insert(obj);

    // 디버그: 현재 read/write set 상태 찍기
    //std::cout << "[DEBUG] after READ in trx" << trx.id << "\n";
    //print_rw_sets(trx);
}

void occ::trx_write(trx_t& trx, object_t obj) {
    occ_timestamp++;
    std::cout << "[LOCAL WRITE] trx" << trx.id << " write object: " << obj.name << "\n";

    // write_set에 추가
    trx.write_set.insert(obj);

    // 디버그: 현재 read/write set 상태 찍기
    //std::cout << "[DEBUG] after WRITE in trx" << trx.id << "\n";
    //print_rw_sets(trx);
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
	// std::cout << "[DEBUG] validate for trx" << trx.id
    //           << " start=" << trx.start_ts
    //           << " validate=" << occ_timestamp
    //           << " finish=" << trx.finish_ts << "\n";
    // print_rw_sets(trx);

	// 1) �� Ʈ������� validate Ÿ�ӽ����� ����
	trx.validate_ts = occ_timestamp;   // �� �̸��� system.h�� ���� ����

	// 2) �ٸ� Ʈ����ǵ�� �浹 ���� �˻�
	for (auto it = trx_map.begin(); it != trx_map.end(); ++it) {
		trx_t& other = it->second;

		if (other.id == trx.id) continue; // �ڱ� �ڽ��� ��ŵ

		// ���� Ŀ�� �� �� Ʈ������� ���� ��󿡼� ����
		// (finish_ts �ʱⰪ�� 0 �Ǵ� -1 ������ system.h ���� ���� ���߼���)
		if (other.finish_ts == INF) continue;

		// (1) other �� trx ���� ���� �̹� �������� ��ġ�� ������ ���� �� OK
		if (other.finish_ts <= trx.start_ts) {
			continue;
		}

		// (2) other �� trx ���� ���Ŀ� �����ߴٸ� ���� ��ġ�� �� ���� �� OK
		if (other.start_ts >= trx.validate_ts) {
			continue;
		}

		bool conflict = false;
		for (const auto& obj : trx.read_set) {
			if (other.write_set.find(obj) != other.write_set.end()) {
				conflict = true;
				break;
			}
		}

		if (conflict) {
			return false;
		}

	}

	// ������� ������ ���� ����
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
		// Ŀ�� �Ϸ� �ð��� ������Ʈ
	trx.finish_ts = occ_timestamp;   // �̸��� system.h�� ���缭 ����
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
        if (actions.empty()) break;

        std::string op = *it;

        // 최소 길이 체크 (tid 뽑을 수 없는 이상한 문자열 방어)
        if (op.size() < 2) {
            std::cout << "WRONG OP FORMAT: " << op << "\n";
            it = actions.erase(it);
            continue;
        }

        char opcode = op[0];        // R / W / C
        int  tid    = op[1] - '0';  // 과제에서 tid 한 자리라고 가정
        char oid    = '\0';

        // R / W 일 때만 object id 필요
        if (opcode == 'R' || opcode == 'W') {
            // 형식: R1(A) / W2(B) ⇒ op[3] 가 객체 이름
            if (op.size() < 4) {
                std::cout << "WRONG OP FORMAT: " << op << "\n";
                it = actions.erase(it);
                continue;
            }
            oid = op[3];
        }

        trx_t& trx = trx_map[tid];

        // start_ts 세팅: 이 트랜잭션이 처음 실행될 때 타임스탬프 부여
        if (trx.start_ts == INF) {          // ctor에서 0으로 초기화돼 있으니까 0 기준
            trx.start_ts = occ_timestamp; // 전역 occ_timestamp 사용
        }

        // step1. 연산 실행
        if (opcode == 'R') {
            trx_read(trx, obj_map[oid]);
        } else if (opcode == 'W') {
            trx_write(trx, obj_map[oid]);
        } else if (opcode == 'C') {
            bool pass = trx_validate(trx);
            if (pass) {
                // write 안에서 실제 commit 까지 처리하도록 설계되어 있음
                write(trx);
            } else {
                abort(trx);
            }
        } else {
            std::cout << "WRONG OPERATION!\n";
        }

        // 이 액션은 처리 끝났으니 삭제
        it = actions.erase(it);
    }

    return;
}
