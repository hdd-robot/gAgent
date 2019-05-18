#include "fipa.hh"

class MTS_i :  public POA_FIPA::MTS, public PortableServer::RefCountServantBase
{
public:
   MTS_i();
   virtual ~MTS_i();
   virtual void message(const ::FIPA::FipaMessage& aFipaMessage);

};
