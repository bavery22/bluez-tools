


#include <stddef.h>  // defines NULL
#include "glibHandler.h"

// Global static pointer used to ensure a single instance of the class.
GlibHandler* GlibHandler::m_pInstance = NULL;  

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
GlibHandler* GlibHandler::Instance()
{
  if (!m_pInstance)   // Only allow one instance of class to be generated.
    m_pInstance = new GlibHandler;

  return m_pInstance;
}
