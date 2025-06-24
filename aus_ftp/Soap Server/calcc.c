#include "calc.nsmap" // XML namespace mapping table (only needed once at the global level)
#include "soapH.h"    // client stubs, serializers, etc.

int main()
{
  struct soap *soap = soap_new(); // allocate and initialize a context
  double sum;
  if (soap_call_ns2__add(soap, NULL, NULL, 1.23, 4.56, &sum) == SOAP_OK)
    printf("Sum = %g\n", sum);
  else
    soap_print_fault(soap, stderr);
  soap_destroy(soap); // delete managed deserialized C++ instances
  soap_end(soap);     // delete other managed data
  soap_free(soap);    // free the soap struct context data
}