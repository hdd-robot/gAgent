
#include <iostream>
#include <CORBA.h>
#include <Naming.hh>


#include "fipa.hh"

/** Name is defined in the server.cpp */
#define SERVER_NAME		"MyServerName"

using namespace std;
using namespace FIPA;


int main(int argc, char ** argv)
{
	try {
		//------------------------------------------------------------------------
		// Initialize ORB object.
		//------------------------------------------------------------------------
		CORBA::ORB_ptr orb = CORBA::ORB_init(argc, argv);

		//------------------------------------------------------------------------
		// Resolve service
		//------------------------------------------------------------------------
		FIPA::MTS_ptr service_server = 0;

		try {

			//------------------------------------------------------------------------
			// Bind ORB object to name service object.
			// (Reference to Name service root context.)
			//------------------------------------------------------------------------
			CORBA::Object_var ns_obj = orb->resolve_initial_references("NameService");

			if (!CORBA::is_nil(ns_obj)) {
				//------------------------------------------------------------------------
				// Bind ORB object to name service object.
				// (Reference to Name service root context.)
				//------------------------------------------------------------------------
				CosNaming::NamingContext_ptr nc = CosNaming::NamingContext::_narrow(ns_obj);
				
				//------------------------------------------------------------------------
				// The "name text" put forth by CORBA server in name service.
				// This same name ("MyServerName") is used by the CORBA server when
				// binding to the name server (CosNaming::Name).
				//------------------------------------------------------------------------
				CosNaming::Name name;
				name.length(1);
				name[0].id = CORBA::string_dup(SERVER_NAME);
				name[0].kind = CORBA::string_dup("");

				//------------------------------------------------------------------------
				// Resolve "name text" identifier to an object reference.
				//------------------------------------------------------------------------
				CORBA::Object_ptr obj = nc->resolve(name);

				if (!CORBA::is_nil(obj)) {
					service_server = FIPA::MTS::_narrow(obj);
				}
			}
		} catch (CosNaming::NamingContext::NotFound &) {
			cerr << "Caught corba not found" << endl;
		} catch (CosNaming::NamingContext::InvalidName &) {
			cerr << "Caught corba invalid name" << endl;
		} catch (CosNaming::NamingContext::CannotProceed &) {
			cerr << "Caught corba cannot proceed" << endl;
		}

		//------------------------------------------------------------------------
		// Do stuff
		//------------------------------------------------------------------------
		if (!CORBA::is_nil(service_server)) {

			FIPA::FipaMessage *msg = new FIPA::FipaMessage;

				URL url = "test";

				AgentID aid ;
				aid.addresses.
				AgentIDs aids;
				aids.AgentIDs(aid);

				Envelope env;
				env.from.OptAgentID(aid);
				env.to.AgentIDs(aids);

				Envelopes envs;
				envs.Envelopes(env);

				FipaMessage fmsg;
				fmsg.messageEnvelopes(envs);



			service_server->message(fmsg);
			//cout << "response from Server: " << server << endl;
			//CORBA::string_free(server);
		}

		//------------------------------------------------------------------------
		// Destroy OBR
   		//------------------------------------------------------------------------
		orb->destroy();

	} catch (CORBA::UNKNOWN) {
		cerr << "Caught CORBA exception: unknown exception" << endl;
	}
}

