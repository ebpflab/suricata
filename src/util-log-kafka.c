
#include "suricata-common.h" /* errno.h, string.h, etc. */
#include "util-log-kafka.h"
#include "util-logopenfile.h"
#include "util-byte.h"
#include "util-debug.h"

#ifdef HAVE_LIBRDKAFKA

static void SCLogFileCloseKafka(LogFileCtx *log_ctx)
{
	SCLogKafkaContext *kafka_ctx = log_ctx->kafka;

	if (kafka_ctx == NULL) {
		return ;
	}

	if (kafka_ctx->rk) {
		/* Poll to handle delivery reports */
		rd_kafka_poll(kafka_ctx->rk, 0);

		/* Wait for messages to be delivered */
		while (rd_kafka_outq_len(kafka_ctx->rk) > 0) {
			rd_kafka_poll(kafka_ctx->rk, 100);
		}
	}

	if (kafka_ctx->rkt) {
		/* Destroy topic */
		rd_kafka_topic_destroy(kafka_ctx->rkt);
	}

	if (kafka_ctx->rk) {
		/* Destroy the handle */
		rd_kafka_destroy(kafka_ctx->rk);
	}

	return ;
}

int LogFileWriteKafka(void *lf_ctx, const char *string, size_t string_len)
{
	LogFileCtx *log_ctx = lf_ctx; 

	SCLogKafkaContext *kafka_ctx = log_ctx->kafka;

	int partition = kafka_ctx->partition % (log_ctx->kafka_setup.partitions);

	if (rd_kafka_produce(kafka_ctx->rkt, partition,
						RD_KAFKA_MSG_F_COPY, (void *)string, string_len, /* Payload and length */
						NULL, 0, /* Optional key and its length */
						/* Message opaque, provided in delivery report callback as msg_opaque. */
						NULL) == -1)
	{
		SCLogError("%% Failed to produce to topic %s " 
					"partition %i: %s\n",
					log_ctx->kafka_setup.topic, partition,
					rd_kafka_err2str(rd_kafka_last_error()));
		/* Poll to handle delivery reports */
		rd_kafka_poll(kafka_ctx->rk, 0);
	}
	kafka_ctx->partition++;

	return -1;
}

static void msg_delivered(rd_kafka_t *rk, 
								void *payload, size_t len, int error_code,
								void *opaque, void *msg_opaque)
{
	rk = rk;
	payload = payload;
	len = len;
	opaque = opaque;
	msg_opaque = msg_opaque;

	if (error_code) {
		SCLogError("%% Message delivery failed: %s\n", rd_kafka_err2str(error_code));
	}
}

int SCConfLogOpenKafka(ConfNode *kafka_node, void *lf_ctx)
{
	LogFileCtx *log_ctx = lf_ctx;
	const char *partitions = NULL;
	SCLogKafkaContext *kafka_ctx = NULL;

	if (kafka_node == NULL) {
		return -1;
	}

	log_ctx->kafka_setup.brokers = ConfNodeLookupChildValue(kafka_node, "brokers");
	log_ctx->kafka_setup.topic = ConfNodeLookupChildValue(kafka_node, "topic");
	partitions =  ConfNodeLookupChildValue(kafka_node, "partitions");
	log_ctx->kafka_setup.partitions = atoi(partitions);

	/*create kafka ctx*/ 
	rd_kafka_conf_t *conf;
	rd_kafka_topic_conf_t *topic_conf;
	char tmp[16];
	char errstr[512];
	kafka_ctx = (SCLogKafkaContext*) SCCalloc(1, sizeof(SCLogKafkaContext));

	if (kafka_ctx == NULL) {
		SCLogError("Unable to allocate kafka context");
		exit(EXIT_FAILURE);
	}

	conf = rd_kafka_conf_new();
	snprintf(tmp, sizeof(tmp), "%i", SIGIO);
	if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(conf, "internal.termination.signal",
							tmp, errstr, sizeof(errstr))) {
		SCLogError("Unable to allocate kafka context");
	}

	if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(conf, "broker.version.fallback",
							"0.8.2", errstr, sizeof(errstr))) {
		SCLogError("%s", errstr);
	}

	if (RD_KAFKA_CONF_OK != rd_kafka_conf_set(conf, "queue.buffering.max.messages",
							"500000", errstr, sizeof(errstr))) {
		SCLogError("%s", errstr);
	}
	rd_kafka_conf_set_dr_cb(conf, msg_delivered);

	if (!(kafka_ctx->rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr)))) {
		SCLogError("%% Failed to create new producer: %s", errstr);
		exit(EXIT_FAILURE);
	}

	if (0 == rd_kafka_brokers_add(kafka_ctx->rk, log_ctx->kafka_setup.brokers)) {
		SCLogError("%% No valid brokers specified");
		exit(EXIT_FAILURE);
	}

	topic_conf = rd_kafka_topic_conf_new();
	kafka_ctx->rkt = rd_kafka_topic_new(kafka_ctx->rk, log_ctx->kafka_setup.topic, topic_conf);

	if (NULL == kafka_ctx->rkt) {
		SCLogError("%% Failed to create kafka topic %s", log_ctx->kafka_setup.topic);
		exit(EXIT_FAILURE);
	}

	kafka_ctx->partition = 0;
	log_ctx->kafka = kafka_ctx;
	log_ctx->Close = SCLogFileCloseKafka;

	return 0;
}

#endif
