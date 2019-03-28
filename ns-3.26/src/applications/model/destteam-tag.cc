#include "destteam-tag.h"
#include "ns3/log.h"

namespace ns3 {


DestTeamTag::DestTeamTag ()
{
    m_DestTeamTag=0;
}

void
DestTeamTag::SetDestTeam(uint16_t pt)
{
  std::cout<<"setdestteam: "<<m_DestTeamTag<<std::endl;
  m_DestTeamTag = pt;
}

uint16_t
DestTeamTag::GetDestTeam() const
{
  return m_DestTeamTag;
}

NS_OBJECT_ENSURE_REGISTERED (DestTeamTag);

TypeId
DestTeamTag::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::DestTeamTag")
    .SetParent<Tag>()
    .AddConstructor<DestTeamTag>()
  ;
  return tid;
}
TypeId
DestTeamTag::GetInstanceTypeId () const
{
  return GetTypeId();
}
uint32_t
DestTeamTag::GetSerializedSize () const
{
  return 2;
}
void
DestTeamTag::Serialize (TagBuffer i) const
{
  i.WriteU16(m_DestTeamTag);
}
void
DestTeamTag::Deserialize (TagBuffer i)
{
  m_DestTeamTag= i.ReadU16();
}
void
DestTeamTag::Print (std::ostream &os) const
{
  os << "DestTeamTag=" << (uint16_t)m_DestTeamTag;
}

} // namespace ns3
