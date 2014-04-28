#ifndef IOTLAB_UID_H_
#define IOTLAB_UID_H_

#include "unique_id.h"

static inline uint16_t iotlab_uid() {
	return (uid->uid8[8] | (uid->uid8[10] << 7)) << 8 | uid->uid8[9];
}

#endif /* IOTLAB_UID_H_ */
