/****************************************************************

  Generated by Eclipse Cyclone DDS IDL to C Translator
  File name: MindosData.h
  Source: /home/li/cyclonedds-install/idl/MindosData.idl
  Cyclone DDS: V0.11.0

*****************************************************************/
#ifndef DDSC_MINDOSDATA_H_1FA67AF800DAD5F71026B1A3C18C3388
#define DDSC_MINDOSDATA_H_1FA67AF800DAD5F71026B1A3C18C3388

#include "dds/ddsc/dds_public_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MindosData_Msg
{
  int32_t dsize;
  char * message;
} MindosData_Msg;

extern const dds_topic_descriptor_t MindosData_Msg_desc;

#define MindosData_Msg__alloc() \
((MindosData_Msg*) dds_alloc (sizeof (MindosData_Msg)));

#define MindosData_Msg_free(d,o) \
dds_sample_free ((d), &MindosData_Msg_desc, (o))

#ifdef __cplusplus
}
#endif

#endif /* DDSC_MINDOSDATA_H_1FA67AF800DAD5F71026B1A3C18C3388 */
