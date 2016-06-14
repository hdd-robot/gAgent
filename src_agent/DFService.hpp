/*
 * DFService.hpp
 *
 *  Created on: 14 août 2014
 *      Author: cyberjunky
 */

#ifndef DFSERVICE_HPP_
#define DFSERVICE_HPP_
#include "Agent.hpp"


namespace gagent
{



  class DFService{
  public:
    DFService ();
    virtual
    ~DFService ();

    void
    setServiceName (std::string);

    void
    setServiceType ();

    void
    setOntologies (const std::string);

    void
    setLanguages (const std::string);

    void
    addProperties (const std::string, const std::string);

  private:
    std::string name;
    std::string type;
    std::string ownership;
    std::string interactionProtocols;
    std::string language;
    std::string proprites;

  };

} /* namespace gagent */

#else
namespace gagent
  {
    class DFService;
  }
#endif /* DFSERVICE_HPP_ */
