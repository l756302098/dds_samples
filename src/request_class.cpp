#include "dds/dds.h"
#include <stdio.h>
#include <stdlib.h>
#include "MindosData.h"
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>

#define MAX_SAMPLES 1
const char *pubPartitions[] = { "ping" };
const char *subPartitions[] = { "pong" };

class Request
{
private:
  dds_entity_t participant,topic;
  dds_entity_t publisher,subscriber,writer,reader;
  dds_entity_t waitSet,rdCond;
  std::mutex mtx;
  std::function<void(MindosData_Msg&)> callback;
public:
  Request(/* args */);
  ~Request();
  bool Start();
  bool Invoke();
  bool Stop();
};

Request::Request(/* args */):callback(nullptr)
{
}

Request::~Request()
{
  
}

bool Request::Stop()
{
  {
    std::lock_guard<std::mutex> lock(mtx);
    callback = nullptr;
  }
  dds_return_t status = dds_delete (participant);
  if (status < 0)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-status));
  return true;
}

bool Request::Invoke()
{
  uint32_t matchStatus = 0;
  dds_return_t rc = dds_get_status_changes (writer, &matchStatus);
  if (rc != DDS_RETCODE_OK)
  {
    DDS_FATAL("dds_get_status_changes: %s\n", dds_strretcode(-rc));
    return false;
  }
  if(!(matchStatus & DDS_PUBLICATION_MATCHED_STATUS))
  {
    printf ("=== [Publisher] Waiting for PUBLICATION_MATCHED... \n");
    fflush (stdout);
    return false;
  }

  void *samples[MAX_SAMPLES];

  MindosData_Msg pub_data;
  MindosData_Msg sub_data[MAX_SAMPLES];
  memset (&sub_data, 0, sizeof (sub_data));
  memset (&pub_data, 0, sizeof (pub_data));
  samples[0] = &sub_data[0];
  dds_sample_info_t infos[MAX_SAMPLES];

  dds_time_t waitTimeout = DDS_SECS (3);

  pub_data.dsize = 10;
  std::string data = "hello world.";
  pub_data.message = const_cast<char*>(data.c_str());
  dds_return_t status = dds_write_ts (writer, &pub_data, dds_time());
  if (status < 0)
  {
    DDS_FATAL("dds_write_ts: %s\n", dds_strretcode(-status));
    /* Clean up */
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
      MindosData_Msg_free (&sub_data[i], DDS_FREE_CONTENTS);
    }
    return false;
  }

  if (!dds_triggered (waitSet))
  {
    printf ("# dds_triggered Waiting ...\n");
    dds_attach_t wsresults[MAX_SAMPLES];
    status = dds_waitset_wait (waitSet, wsresults, MAX_SAMPLES, waitTimeout);
    if (status < 0)
      DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-status));
    printf ("# dds_waitset_wait finish.%d\n",status);
    if (status != 0) {
      status = dds_take (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
      if (status < 0)
        DDS_FATAL("dds_take: %s\n", dds_strretcode(-status));
      MindosData_Msg *msg = (MindosData_Msg*) samples[0];
      printf ("=== [Subscriber] Received : ");
      printf ("Message (%i, %s)\n", msg->dsize, msg->message);
      fflush (stdout);
      std::lock_guard<std::mutex> lock(mtx);
      if(callback!=nullptr)
        callback(*msg);
    }
  }
  return true;
}

bool Request::Start()
{
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
  {
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
    return false;
  }
  /* A DDS_Topic is created for our sample type on the domain participant. */
  dds_qos_t *tQos = dds_create_qos ();
  dds_qset_reliability (tQos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  topic = dds_create_topic (participant, &MindosData_Msg_desc, "RoundTrip", tQos, NULL);
  if (topic < 0)
  {
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
    return false;
  }
  dds_delete_qos (tQos);

  /* A DDS_Publisher is created on the domain participant. */
  dds_qos_t *pubQos = dds_create_qos ();
  dds_qset_partition (pubQos, 1, pubPartitions);

  publisher = dds_create_publisher (participant, pubQos, NULL);
  if (publisher < 0)
  {
    DDS_FATAL("dds_create_publisher: %s\n", dds_strretcode(-publisher));
    return false;
  }
  dds_delete_qos (pubQos);

  /* A DDS_DataWriter is created on the Publisher & Topic with a modified Qos. */
  dds_qos_t *wQos = dds_create_qos ();
  dds_qset_writer_data_lifecycle (wQos, false);
  writer = dds_create_writer (publisher, topic, wQos, NULL);
  if (writer < 0)
  {
    DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));
    return false;
  }
  dds_delete_qos (wQos);

  /* A DDS_Subscriber is created on the domain participant. */
  dds_qos_t *subQos = dds_create_qos ();

  dds_qset_partition (subQos, 1, subPartitions);

  subscriber = dds_create_subscriber (participant, subQos, NULL);
  if (subscriber < 0)
    DDS_FATAL("dds_create_subscriber: %s\n", dds_strretcode(-subscriber));
  dds_delete_qos (subQos);
  /* A DDS_DataReader is created on the Subscriber & Topic with a modified QoS. */
  reader = dds_create_reader (subscriber, topic, NULL, NULL);
  if (reader < 0)
  {
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
    return false;
  }

  waitSet = dds_create_waitset (participant);
  rdCond = dds_create_readcondition (reader, DDS_ANY_STATE);
  dds_return_t status = dds_waitset_attach (waitSet, rdCond, reader);
  if (status < 0)
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));
  status = dds_waitset_attach (waitSet, waitSet, waitSet);
  if (status < 0)
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));

  return true;
}



int main (int argc, char ** argv)
{
  Request rq;
  rq.Start();
  while (1)
  {
    std::cout << "Invoke1..." << std::endl;
    rq.Invoke();
    std::cout << "Invoke2..." << std::endl;
    sleep(1);
  }
  
  rq.Stop();

  return EXIT_SUCCESS;
}

