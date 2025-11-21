#ifndef OCC_H
#define OCC_H

#include "system.h"

class occ {
public:
	occ();

	void trx_read(trx_t& trx, object_t obj);
	void trx_write(trx_t& trx, object_t obj);
	bool trx_validate(trx_t& trx);
	void commit(trx_t& trx);
	void abort(trx_t& trx);
	void write(trx_t&);
	void run();

};

#endif
