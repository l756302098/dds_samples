#include "dds/dds.h"
#include "MindosData.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <functional>
#include <atomic>
#include <signal.h>

#define MAX_SAMPLES 1

class Subscriber
{
private:
  std::string topicName;
  std::atomic_bool isReceive;
  std::mutex mtx;
  //dds
  dds_entity_t participant,topic,reader;
  dds_entity_t waitSet,readCond;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  //func
  std::function<void(MindosData_Msg&)> callback;
public:
  explicit Subscriber(const std::string& name);
  ~Subscriber();
  void SetTrigger();
  bool Start();
  bool Stop();
};

Subscriber::Subscriber(const std::string& name):topicName(name),isReceive(false)
{
  std::cout << "topic name:" << name << std::endl;
  samples[0] = MindosData_Msg__alloc ();
}

Subscriber::~Subscriber()
{
  SetTrigger();
  MindosData_Msg_free (samples[0], DDS_FREE_ALL);
  std::cout << "samples free finish." << std::endl;
}

void Subscriber::SetTrigger()
{
  std::cout << "Subscriber SetTrigger" << std::endl;
  dds_waitset_set_trigger (waitSet, true);
}

bool Subscriber::Start()
{
  /* Create a Participant. */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
  {
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));
    return false;
  }

  /* Create a Topic. */
  topic = dds_create_topic (
  participant, &MindosData_Msg_desc, topicName.c_str(), NULL, NULL);
  if (topic < 0)
  {
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));
    return false;
  }

  /* Create a reliable Reader. */
  dds_qos_t *qos;
  qos = dds_create_qos ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  reader = dds_create_reader (participant, topic, qos, NULL);
  if (reader < 0)
  {
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
    return false;
  }
  dds_delete_qos(qos);

  printf ("\n=== [Subscriber] Waiting for a sample ...\n");
  fflush (stdout);
  waitSet = dds_create_waitset (participant);
  readCond = dds_create_readcondition (reader, DDS_ANY_STATE);
  dds_return_t status = dds_waitset_attach (waitSet, readCond, reader);
  if (status < 0)
  {
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));
    return false;
  }
  status = dds_waitset_attach (waitSet, waitSet, waitSet);
  if (status < 0)
  {
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));
    return false;
  }

  // if (!dds_triggered (waitSet))
  // {
  //   printf("# Warm up complete.\n\n");
  //   fflush (stdout);
  // }

  std::cout << "samples alloc finish." << std::endl;
  isReceive = true;
  std::thread recvTh([&](){
    std::cout << "start thread:" << std::this_thread::get_id() << std::endl;
    
    while (!dds_triggered (waitSet) && isReceive)
    //while (!dds_triggered (waitSet))
    {
      printf ("# dds_triggered Waiting ...\n");
      dds_attach_t wsresults[1];
      status = dds_waitset_wait (waitSet, wsresults, 1U, DDS_SECS (3));
      if (status < 0)
      {
        DDS_FATAL("dds_waitset_wait: %s\n", dds_strretcode(-status));
        break;
      }
      printf ("# dds_waitset_wait finish.%d\n",status);
      if (status != 0) {
        //deal data
        status = dds_take (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
        if (status < 0)
        {
          DDS_FATAL("dds_take: %s\n", dds_strretcode(-status));
          break;
        }
        MindosData_Msg *msg;
        msg = (MindosData_Msg*) samples[0];
        {
          std::lock_guard<std::mutex> lock(mtx);
          if(callback!=nullptr)
            callback(*msg);
        }
      }
      else
      {
        printf ("# status is 0.\n");
      }
    }
    printf ("# thread exit.\n");
  });
  recvTh.detach();
  return true;
}

bool Subscriber::Stop()
{
  /* Free the data location. */
  {
    std::lock_guard<std::mutex> lock(mtx);
    callback = nullptr;
  }   
  isReceive =  false;
  SetTrigger();
  /* Deleting the participant will delete all its children recursively as well. */
  dds_return_t rc = dds_delete (participant);
  if (rc != DDS_RETCODE_OK)
  {
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
    return false;
  }
  return true;
}

std::atomic_bool running;
Subscriber sub("/data");
void CtrlHandler (int sig)
{
  std::cout << "CtrlHandler." << std::endl;
  running = false;
  sub.SetTrigger();
}

int main (int argc, char ** argv)
{
  struct sigaction sat, oldAction;
  sat.sa_handler = CtrlHandler;
  sigemptyset (&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
  running = true;

  sub.Start();
  while (running)
  {
    std::cout << "sleep..." << std::endl;
    sleep(1);
  }

  sub.Stop();

  return EXIT_SUCCESS;
}

