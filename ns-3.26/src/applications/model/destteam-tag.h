
#ifndef DEST_TEAM_TAG_H
#define DEST_TEAM_TAG_H

#include "ns3/tag.h"

namespace ns3 {

/**
 * \ingroup aqua-sim-ng
 *
 * \brief Generic packet tag used to assist in declaring protocols used for each packet
 */
class DestTeamTag : public Tag
{
public:
  DestTeamTag();

  void SetDestTeam(uint16_t tag);
  uint16_t GetDestTeam() const;

  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;



private:
  uint16_t m_DestTeamTag;
};

};

#endif
