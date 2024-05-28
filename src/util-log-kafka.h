#ifndef __UTIL_LOG_KAFKA_H__
#define __UTIL_LOG_KAFKA_H__

#ifdef HAVE_LIBRDKAFKA
#include <librdkafka/rdkafka.h>
#include "conf.h"            /* ConfNode   */

typedef struct KafkaSetup_{
	char *brokers;
	char *topic;
	int partitions;
}KafkaSetup;

typedef struct SCLogKafkaContext_ {
    rd_kafka_t *rk;
	rd_kafka_topic_t *rkt;
	long partition;
} SCLogKafkaContext;

int SCConfLogOpenKafka(ConfNode *, void *);
int LogFileWriteKafka(void *, const char *, size_t);
#endif

#endif /* __UTIL_LOG_KAFKA_H__ */

