/****************************************************************

  Generated by Eclipse Cyclone DDS IDL to C Translator
  File name: MindosData.c
  Source: /home/li/cyclonedds-install/idl/MindosData.idl
  Cyclone DDS: V0.11.0

*****************************************************************/
#include "MindosData.h"

static const uint32_t MindosData_Msg_ops [] =
{
  /* Msg */
  DDS_OP_ADR | DDS_OP_TYPE_4BY | DDS_OP_FLAG_SGN, offsetof (MindosData_Msg, dsize),
  DDS_OP_ADR | DDS_OP_TYPE_STR, offsetof (MindosData_Msg, message),
  DDS_OP_RTS
};

/* Type Information:
  [MINIMAL 894d8d4dec8430b660f9699ae66f] (#deps: 0)
  [COMPLETE e30c7679dd9385bfbbd207625d6f] (#deps: 0)
*/
#define TYPE_INFO_CDR_MindosData_Msg (const unsigned char []){ \
  0x60, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x40, 0x28, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, \
  0x14, 0x00, 0x00, 0x00, 0xf1, 0x89, 0x4d, 0x8d, 0x4d, 0xec, 0x84, 0x30, 0xb6, 0x60, 0xf9, 0x69, \
  0x9a, 0xe6, 0x6f, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, \
  0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x40, 0x28, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, \
  0x14, 0x00, 0x00, 0x00, 0xf2, 0xe3, 0x0c, 0x76, 0x79, 0xdd, 0x93, 0x85, 0xbf, 0xbb, 0xd2, 0x07, \
  0x62, 0x5d, 0x6f, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, \
  0x00, 0x00, 0x00, 0x00\
}
#define TYPE_INFO_CDR_SZ_MindosData_Msg 100u
#define TYPE_MAP_CDR_MindosData_Msg (const unsigned char []){ \
  0x4c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf1, 0x89, 0x4d, 0x8d, 0x4d, 0xec, 0x84, 0x30, \
  0xb6, 0x60, 0xf9, 0x69, 0x9a, 0xe6, 0x6f, 0x00, 0x34, 0x00, 0x00, 0x00, 0xf1, 0x51, 0x01, 0x00, \
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, \
  0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x23, 0xe1, 0xf0, 0xc1, 0x00, \
  0x0c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x70, 0x00, 0x78, 0xe7, 0x31, 0x02, \
  0x72, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf2, 0xe3, 0x0c, 0x76, 0x79, 0xdd, 0x93, 0x85, \
  0xbf, 0xbb, 0xd2, 0x07, 0x62, 0x5d, 0x6f, 0x00, 0x5a, 0x00, 0x00, 0x00, 0xf2, 0x51, 0x01, 0x00, \
  0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x4d, 0x69, 0x6e, 0x64, \
  0x6f, 0x73, 0x44, 0x61, 0x74, 0x61, 0x3a, 0x3a, 0x4d, 0x73, 0x67, 0x00, 0x36, 0x00, 0x00, 0x00, \
  0x02, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, \
  0x06, 0x00, 0x00, 0x00, 0x64, 0x73, 0x69, 0x7a, 0x65, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, \
  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x70, 0x00, 0x08, 0x00, 0x00, 0x00, 0x6d, 0x65, 0x73, 0x73, \
  0x61, 0x67, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, \
  0xf2, 0xe3, 0x0c, 0x76, 0x79, 0xdd, 0x93, 0x85, 0xbf, 0xbb, 0xd2, 0x07, 0x62, 0x5d, 0x6f, 0xf1, \
  0x89, 0x4d, 0x8d, 0x4d, 0xec, 0x84, 0x30, 0xb6, 0x60, 0xf9, 0x69, 0x9a, 0xe6, 0x6f\
}
#define TYPE_MAP_CDR_SZ_MindosData_Msg 238u
const dds_topic_descriptor_t MindosData_Msg_desc =
{
  .m_size = sizeof (MindosData_Msg),
  .m_align = dds_alignof (MindosData_Msg),
  .m_flagset = DDS_TOPIC_XTYPES_METADATA,
  .m_nkeys = 0u,
  .m_typename = "MindosData::Msg",
  .m_keys = NULL,
  .m_nops = 3,
  .m_ops = MindosData_Msg_ops,
  .m_meta = "",
  .type_information = { .data = TYPE_INFO_CDR_MindosData_Msg, .sz = TYPE_INFO_CDR_SZ_MindosData_Msg },
  .type_mapping = { .data = TYPE_MAP_CDR_MindosData_Msg, .sz = TYPE_MAP_CDR_SZ_MindosData_Msg }
};

