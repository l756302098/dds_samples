#include "dds/dds.h"
#include <stdio.h>
#include <stdlib.h>
#include "MindosData.h"
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <string>
#include <iostream>

#define MAX_SAMPLES 10
#define US_IN_ONE_SEC 1000000LL

static dds_entity_t writer;
static dds_entity_t reader;
static dds_entity_t participant;
static dds_entity_t readCond;

static MindosData_Msg pub_data;
static MindosData_Msg sub_data[MAX_SAMPLES];
static void *samples[MAX_SAMPLES];
static dds_sample_info_t info[MAX_SAMPLES];

static dds_entity_t waitSet;
static bool warmUp = true;

static dds_time_t startTime;

static void data_available(dds_entity_t rd, void *arg)
{
  printf("data_available.\n");
  int status;
  (void)arg;
  /* Take sample and check that it is valid */
  status = dds_take (rd, samples, info, MAX_SAMPLES, MAX_SAMPLES);
  if (status < 0)
    DDS_FATAL("dds_take: %s\n", dds_strretcode(-status));
  MindosData_Msg *msg;
  msg = (MindosData_Msg*) samples[0];
  printf ("=== [receive] Received : ");
  printf ("Message (%"PRId32", %s)\n", msg->dsize, msg->message);
  dds_time_t preWriteTime = dds_time();
  status = dds_write_ts (writer, &pub_data, preWriteTime);
  if (status < 0)
    DDS_FATAL("dds_write_ts: %s\n", dds_strretcode(-status));
}

static dds_entity_t prepare_dds(dds_entity_t *wr, dds_entity_t *rd, dds_entity_t *rdcond, dds_listener_t *listener)
{
  dds_return_t status;
  dds_entity_t topic;
  dds_entity_t publisher;
  dds_entity_t subscriber;

  const char *pubPartitions[] = { "ping" };
  const char *subPartitions[] = { "pong" };
  dds_qos_t *pubQos;
  dds_qos_t *subQos;
  dds_qos_t *tQos;
  dds_qos_t *wQos;

  /* A DDS_Topic is created for our sample type on the domain participant. */
  tQos = dds_create_qos ();
  dds_qset_reliability (tQos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  topic = dds_create_topic (participant, &MindosData_Msg_desc, "RoundTrip", tQos, NULL);
  if (topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
  dds_delete_qos (tQos);

  /* A DDS_Publisher is created on the domain participant. */
  pubQos = dds_create_qos ();
  dds_qset_partition (pubQos, 1, pubPartitions);

  publisher = dds_create_publisher (participant, pubQos, NULL);
  if (publisher < 0)
    DDS_FATAL("dds_create_publisher: %s\n", dds_strretcode(-publisher));
  dds_delete_qos (pubQos);

  /* A DDS_DataWriter is created on the Publisher & Topic with a modified Qos. */
  wQos = dds_create_qos ();
  dds_qset_writer_data_lifecycle (wQos, false);
  *wr = dds_create_writer (publisher, topic, wQos, NULL);
  if (*wr < 0)
    DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-*wr));
  dds_delete_qos (wQos);

  /* A DDS_Subscriber is created on the domain participant. */
  subQos = dds_create_qos ();

  dds_qset_partition (subQos, 1, subPartitions);

  subscriber = dds_create_subscriber (participant, subQos, NULL);
  if (subscriber < 0)
    DDS_FATAL("dds_create_subscriber: %s\n", dds_strretcode(-subscriber));
  dds_delete_qos (subQos);
  /* A DDS_DataReader is created on the Subscriber & Topic with a modified QoS. */
  *rd = dds_create_reader (subscriber, topic, NULL, listener);
  if (*rd < 0)
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-*rd));

  waitSet = dds_create_waitset (participant);
  if (listener == NULL) {
    *rdcond = dds_create_readcondition (*rd, DDS_ANY_STATE);
    status = dds_waitset_attach (waitSet, *rdcond, *rd);
    if (status < 0)
      DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));
  } else {
    *rdcond = 0;
  }
  status = dds_waitset_attach (waitSet, waitSet, waitSet);
  if (status < 0)
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));

  return participant;
}

static void finalize_dds(dds_entity_t ppant)
{
  dds_return_t status;
  status = dds_delete (ppant);
  if (status < 0)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-status));
}


int main (int argc, char ** argv)
{
  uint64_t numSamples = 0;
  dds_time_t difference = 0;

  dds_attach_t wsresults[1];
  size_t wsresultsize = 1U;
  dds_time_t waitTimeout = DDS_SECS (1);
  unsigned long i;
  int status;

  startTime = dds_time ();

  dds_listener_t *listener = NULL;

  memset (&sub_data, 0, sizeof (sub_data));
  memset (&pub_data, 0, sizeof (pub_data));

  for (i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = &sub_data[i];
  }

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

  prepare_dds(&writer, &reader, &readCond, listener);

  pub_data.dsize = 10;
  std::string data = "hello world.";
  pub_data.message = const_cast<char*>(data.c_str());
  printf ("# Waiting for startup jitter to stabilise\n");
  fflush (stdout);
  /* Write a sample that pong can send back */
  while (!dds_triggered (waitSet) && difference < DDS_SECS(5))
  {
    printf ("# dds_triggered Waiting ...\n");
    status = dds_waitset_wait (waitSet, wsresults, wsresultsize, waitTimeout);
    if (status < 0)
      DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-status));
    printf("dds_waitset_wait.\n");
    if (status > 0 && listener == NULL) /* data */
    {
      status = dds_take (reader, samples, info, MAX_SAMPLES, MAX_SAMPLES);
      if (status < 0)
        DDS_FATAL("dds_take: %s\n", dds_strretcode(-status));
    }
    dds_time_t time = dds_time ();
    difference = time - startTime;
  }
  if (!dds_triggered (waitSet))
  {
    warmUp = false;
    printf("# Warm up complete.\n\n");
    printf("# Latency measurements (in us)\n");
    printf("#             Latency [us]                                   Write-access time [us]       Read-access time [us]\n");
    printf("# Seconds     Count   median      min      99%%      max      Count   median      min      Count   median      min\n");
    fflush (stdout);
  }

   /* Write a sample that pong can send back */
  dds_time_t preWriteTime = dds_time();
  status = dds_write_ts (writer, &pub_data, preWriteTime);
  if (status < 0)
    DDS_FATAL("dds_write_ts: %s\n", dds_strretcode(-status));
  std::cout << "dds_write_ts finish." << status << std::endl;
  for (i = 0; !dds_triggered (waitSet) && (!numSamples || i < numSamples); i++)
  {
    printf ("# dds_triggered Waiting ...\n");
    status = dds_waitset_wait (waitSet, wsresults, wsresultsize, waitTimeout);
    if (status < 0)
      DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-status));
    printf ("# dds_waitset_wait finish.%d\n",status);
    if (status != 0 && listener == NULL) {
      data_available(reader, NULL);
    }
  }

  if (!warmUp)
  {
    printf("# Warm up complete.\n\n");
  }

  finalize_dds(participant);

  /* Clean up */
  for (i = 0; i < MAX_SAMPLES; i++)
  {
    MindosData_Msg_free (&sub_data[i], DDS_FREE_CONTENTS);
  }
  MindosData_Msg_free (&pub_data, DDS_FREE_CONTENTS);
  
  return EXIT_SUCCESS;
}

