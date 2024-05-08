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


#define MAX_SAMPLES 10
void *samples[MAX_SAMPLES];
dds_sample_info_t infos[MAX_SAMPLES];

dds_attach_t wsresults[1];
size_t wsresultsize = 1U;
dds_time_t waitTimeout = DDS_SECS (1);

static void data_available(dds_entity_t rd, void *arg)
{
  printf("data_available.\n");
  int status;
  (void)arg;
  /* Take sample and check that it is valid */
  status = dds_take (rd, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
  if (status < 0)
    DDS_FATAL("dds_take: %s\n", dds_strretcode(-status));
  MindosData_Msg *msg;
  msg = (MindosData_Msg*) samples[0];
  printf ("=== [receive] Received : ");
  printf ("Message (%"PRId32", %s)\n", msg->dsize, msg->message);
}

std::atomic_bool running;
void CtrlHandler (int sig)
{
  std::cout << "CtrlHandler." << std::endl;
  running = false;
}

int main (int argc, char ** argv)
{
  struct sigaction sat, oldAction;
  sat.sa_handler = CtrlHandler;
  sigemptyset (&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
  running = true;

  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t reader;
  MindosData_Msg *msg;
  dds_return_t rc;
  dds_qos_t *qos;
  dds_entity_t waitSet;
  dds_listener_t *listener = NULL;
  dds_entity_t readCond;
  dds_return_t status;
  (void)argc;
  (void)argv;

  /* Create a Participant. */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

  /* Create a Topic. */
  topic = dds_create_topic (
    participant, &MindosData_Msg_desc, "MindosData_Msg", NULL, NULL);
  if (topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));

  /* Create a reliable Reader. */
  qos = dds_create_qos ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  reader = dds_create_reader (participant, topic, qos, NULL);
  if (reader < 0)
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
  dds_delete_qos(qos);

  printf ("\n=== [Subscriber] Waiting for a sample ...\n");
  fflush (stdout);

  /* Initialize sample buffer, by pointing the void pointer within
   * the buffer array to a valid sample memory location. */
  samples[0] = MindosData_Msg__alloc ();

  waitSet = dds_create_waitset (participant);
  if (listener == NULL) {
    readCond = dds_create_readcondition (reader, DDS_ANY_STATE);
    status = dds_waitset_attach (waitSet, readCond, reader);
    if (status < 0)
      DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));
  } else {
    readCond = 0;
  }
  status = dds_waitset_attach (waitSet, waitSet, waitSet);
  if (status < 0)
    DDS_FATAL("dds_waitset_attach: %s\n", dds_strretcode(-status));

  if (!dds_triggered (waitSet))
  {
    printf("# Warm up complete.\n\n");
    fflush (stdout);
  }

  std::thread t([&](){
    while (!dds_triggered (waitSet) && running)
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
    printf ("# thread exit.\n");
  });
  t.join();
  printf ("# main exit.\n");
  /* Free the data location. */
  MindosData_Msg_free (samples[0], DDS_FREE_ALL);

  /* Deleting the participant will delete all its children recursively as well. */
  rc = dds_delete (participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

  return EXIT_SUCCESS;
}

