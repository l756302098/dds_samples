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


#define MAX_SAMPLES 3
const char *pubPartitions[] = { "pong" };
const char *subPartitions[] = { "ping" };

class Response
{
private:
  dds_entity_t participant,topic;
  dds_entity_t publisher,subscriber,writer,reader;
  dds_entity_t waitSet,rdCond;
  std::atomic_bool isReceive;
  std::shared_ptr<std::thread> recvTh;
  std::mutex mtx;
  std::function<void(MindosData_Msg&)> callback;
  //data
  MindosData_Msg data[MAX_SAMPLES];
  void * samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  dds_time_t waitTimeout = DDS_INFINITY;
public:
  Response(/* args */);
  ~Response();
  bool Start();
  bool Stop();
  void SetTrigger();
};

Response::Response(/* args */):callback(nullptr)
{
  /* Initialize sample data */
  memset (data, 0, sizeof (data));
  for (size_t i = 0; i < MAX_SAMPLES; i++)
  {
    samples[i] = &data[i];
  }
}

Response::~Response()
{
  /* Clean up */
  SetTrigger();
}

void Response::SetTrigger()
{
  std::cout << "Response SetTrigger" << std::endl;
  isReceive = false;
  dds_waitset_set_trigger (waitSet, true);
}

bool Response::Start()
{
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
  {
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
    return false;
  }
  dds_qos_t *qos = dds_create_qos ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
  topic = dds_create_topic (participant, &MindosData_Msg_desc, "RoundTrip", qos, NULL);
  if (topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
  dds_delete_qos (qos);

  /* A DDS Publisher is created on the domain participant. */

  qos = dds_create_qos ();
  dds_qset_partition (qos, 1, pubPartitions);

  publisher = dds_create_publisher (participant, qos, NULL);
  if (publisher < 0)
    DDS_FATAL("dds_create_publisher: %s\n", dds_strretcode(-publisher));
  dds_delete_qos (qos);

  /* A DDS DataWriter is created on the Publisher & Topic with a modififed Qos. */

  qos = dds_create_qos ();
  dds_qset_writer_data_lifecycle (qos, false);
  writer = dds_create_writer (publisher, topic, qos, NULL);
  if (writer < 0)
    DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));
  dds_delete_qos (qos);

  /* A DDS Subscriber is created on the domain participant. */

  qos = dds_create_qos ();
  dds_qset_partition (qos, 1, subPartitions);

  subscriber = dds_create_subscriber (participant, qos, NULL);
  if (subscriber < 0)
    DDS_FATAL("dds_create_subscriber: %s\n", dds_strretcode(-subscriber));
  dds_delete_qos (qos);

  /* A DDS DataReader is created on the Subscriber & Topic with a modified QoS. */

  reader = dds_create_reader (subscriber, topic, NULL, NULL);
  if (reader < 0)
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));

  waitSet = dds_create_waitset (participant);
  rdCond = dds_create_readcondition (reader, DDS_ANY_STATE);
  dds_return_t status = dds_waitset_attach (waitSet, rdCond, reader);
  if (status < 0)
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));
  status = dds_waitset_attach (waitSet, waitSet, waitSet);
  if (status < 0)
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));

  printf ("Waiting for samples from ping to send back...\n");
  fflush (stdout);

  //receive data
  isReceive = true;
  recvTh = std::make_shared<std::thread>([&](){
    std::cout << "start thread:" << std::this_thread::get_id() << std::endl;
    dds_attach_t wsresults[MAX_SAMPLES];
    while (!dds_triggered (waitSet) && isReceive)
    {
      printf ("# dds_triggered Waiting ...\n");
      dds_return_t status = dds_waitset_wait (waitSet, wsresults, MAX_SAMPLES, waitTimeout);
      if (status < 0)
      {
        DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-status));
        break;
      }
      printf ("# dds_waitset_wait finish.%d\n",status);
      if (status != 0) {
        //deal data
        dds_return_t count = dds_take (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (status < 0)
        {
          DDS_FATAL("dds_take: %s\n", dds_strretcode(-status));
          break;
        }
        for (int i = 0; i < count; i++)
        {
          MindosData_Msg *msg;
          msg = (MindosData_Msg*) samples[i];
          printf ("=== [receive] Received : ");
          printf ("Message (%i, %s)\n", msg->dsize, msg->message);
          fflush (stdout);
          {
            std::lock_guard<std::mutex> lock(mtx);
            if(callback!=nullptr)
              callback(*msg);
          }
        }
      }
    }
    printf ("# thread exit.\n");
  });
  recvTh->detach();

  return true;
}

bool Response::Stop()
{
  std::cout << "Response Stop" << std::endl;
  {
    std::lock_guard<std::mutex> lock(mtx);
    callback = nullptr;
  }
  SetTrigger();
  if(recvTh!=nullptr && recvTh->joinable())
  {
     std::cout << "recvTh join" << std::endl;
    recvTh->join();
     std::cout << "recvTh join finish." << std::endl;
  }
  dds_return_t status = dds_delete (participant);
  if (status < 0)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-status));
  return true;
}

std::atomic_bool running;
Response resp;
void CtrlHandler (int sig)
{
  running = false;
  resp.SetTrigger();
}

int main (int argc, char ** argv)
{
  struct sigaction sat, oldAction;
  sat.sa_handler = CtrlHandler;
  sigemptyset (&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
  running = true;

  resp.Start();
  while (running)
  {
    std::cout << "sleep..." << std::endl;
    sleep(1);
  }
  
  resp.Stop();



  return EXIT_SUCCESS;
}

